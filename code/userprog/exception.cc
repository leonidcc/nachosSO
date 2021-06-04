/// Entry points into the Nachos kernel from user programs.
///
/// There are two kinds of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.  Right now, the only function we support is `Halt`.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
/// For now, this only handles the `Halt` system call.  Everything else core-
/// dumps.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "transfer.hh"
#include "syscall.h"
#include "filesys/directory_entry.hh"
#include "threads/system.hh"

#include <stdio.h>

void InitProcess(void *args) {
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();
    machine->Run();
}

static void
IncrementPC()
{
    unsigned pc;

    pc = machine->ReadRegister(PC_REG);
    machine->WriteRegister(PREV_PC_REG, pc);
    pc = machine->ReadRegister(NEXT_PC_REG);
    machine->WriteRegister(PC_REG, pc);
    pc += 4;
    machine->WriteRegister(NEXT_PC_REG, pc);
}

/// Do some default behavior for an unexpected exception.
///
/// NOTE: this function is meant specifically for unexpected exceptions.  If
/// you implement a new behavior for some exception, do not extend this
/// function: assign a new handler instead.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
static void
DefaultHandler(ExceptionType et)
{
    int exceptionArg = machine->ReadRegister(2);

    fprintf(stderr, "Unexpected user mode exception: %s, arg %d.\n",
            ExceptionTypeToString(et), exceptionArg);
    ASSERT(false);
}

/// Handle a system call exception.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
///
/// The calling convention is the following:
///
/// * system call identifier in `r2`;
/// * 1st argument in `r4`;
/// * 2nd argument in `r5`;
/// * 3rd argument in `r6`;
/// * 4th argument in `r7`;
/// * the result of the system call, if any, must be put back into `r2`.
///
/// And do not forget to increment the program counter before returning. (Or
/// else you will loop making the same system call forever!)
static void
SyscallHandler(ExceptionType _et)
{
    int scid = machine->ReadRegister(2);

    switch (scid) {

        case SC_HALT:
            DEBUG('e', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            break;

        case SC_CREATE: {
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0) {
                DEBUG('e', "Error: address to filename string is null.\n");
                machine->WriteRegister(2, -1);
                break;
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr,
                                    filename, sizeof filename)) {
                DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
                machine->WriteRegister(2, -1);
                break;
            }

            DEBUG('e', "`Create` requested for file `%s`.\n", filename);
            if (fileSystem->Create(filename, 1000))
                machine->WriteRegister(2, 0);
            else {
                DEBUG('e', "Error: could not create file `%s`.\n", filename);
                machine->WriteRegister(2, -1);
            }
            break;
        }

        case SC_REMOVE: {
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0) {
                DEBUG('e', "Error: address to filename string is null.\n");
                machine->WriteRegister(2, -1);
                break;
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr,
                                    filename, sizeof filename)) {
                DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
              machine->WriteRegister(2, -1);
              break;
            }

            DEBUG('e', "`Remove` requested for file `%s`.\n", filename);
            if (fileSystem->Remove(filename))
                 machine->WriteRegister(2, 0);

             else {
                 DEBUG('e', "Error: could not remove file `%s`.\n", filename);
                 machine->WriteRegister(2, -1);
             }
             break;
        }

        case SC_EXEC: {
            int file_name_addr = machine->ReadRegister(4);
            char *file_name = new char[FILE_NAME_MAX_LEN + 1];
            if(!ReadStringFromUser(file_name_addr, file_name, FILE_NAME_MAX_LEN)) {
                DEBUG('e', "Invalid filename");
                machine->WriteRegister(2, -1);
                break;
            }
            OpenFile *exe = fileSystem->Open(file_name);
            if(exe == nullptr) {
                DEBUG('e', "File cannot be opened");
                machine->WriteRegister(2, -1);
                break;
            }
            Thread *t = new Thread(file_name, true, currentThread->GetPriority());
            AddressSpace *space = new AddressSpace(exe);
            t->space = space;
            delete exe;
            DEBUG('e', "Fork sobre thread %s", file_name);
            t->Fork(InitProcess, nullptr);
            machine->WriteRegister(2, t->PID);
            break;
        }
        case SC_JOIN: {
            SpaceId id = machine->ReadRegister(4);
            if(!threads->HasKey(id)) {
                DEBUG('e', "Thread not found");
                break;
            }
            Thread *c = threads->Get(id);
            c->Join();
            break;
        }

        case SC_WRITE: {
                 int userString = machine->ReadRegister(4);
                 if (userString == 0) {
                     DEBUG('e', "Error: address to user string is null.\n");
                     machine->WriteRegister(2, -1);
                     break;
                 }
                 int size = machine->ReadRegister(5);
                 if (size <= 0) {
                     DEBUG('e', "Error: size for Write must be greater than 0.\n");
                     machine->WriteRegister(2, -1);
                     break;
                 }

                 OpenFileId fid = machine->ReadRegister (6);
                 if (fid < 0) {
                     DEBUG('e', "Error: file id must be greater than or equal to 0.\n");
                     machine->WriteRegister(2, -1);
                     break;
                 }

                 char tempString[size + 1];
                 ReadBufferFromUser(userString, tempString, size);
                 tempString[size] = '\0';
                 int bytesWritten = 0;

                 if (fid == CONSOLE_OUTPUT) {
                     DEBUG('e', "`Write` requested to console output.\n");
                     for (; bytesWritten < size; bytesWritten++) {
                        char ch = tempString[bytesWritten];
                        synchConsole->WriteChar(ch);
                    }
                     // machine->WriteRegister(2, -1);
                 }
                 else {
                     DEBUG('e', "`Write` requested to file with id %u.\n", fid);
                     OpenFile *file = currentThread->Files->Get(fid);
                     if (file != nullptr)
                         bytesWritten = file->Write(tempString, size);
                     else {
                         DEBUG('e', "Error: could not open file with id %u for writting.\n",
                               fid);
                         machine->WriteRegister(2, -1);
                         break;
                     }
                 }

                 if (bytesWritten == size)
                     machine->WriteRegister(2, 0);
                 else
                     machine->WriteRegister(2, -1);
                 break;
             }

        case SC_READ:{

            int bufferPointer = machine->ReadRegister(4);
            if (bufferPointer == 0) {
                DEBUG('e', "`Error`: buffer pointer is null.\n");
                machine->WriteRegister(2, -1);
                break;
            }
            int size = machine->ReadRegister(5);
            if (size <= 0) {
                DEBUG('e', "`Error`: size zero or negative.\n");
                machine->WriteRegister(2, -1);
                break;
            }
            OpenFileId id = machine->ReadRegister(6);
            if (id < 0) {
                DEBUG('e', "`Error`: OpenFileId negative.\n");
                machine->WriteRegister(2, -1);
                break;
            }
            char temp[size +1];
            int bytesRead = 0;
            if(id == CONSOLE_INPUT ){
                DEBUG('e', "`Read` requested from console input.\n");
                // implementar leer de la conssola
                for (; bytesRead < size; bytesRead++) {
                    char ch = synchConsole->ReadChar();
                    temp[bytesRead] = ch;
                    if (ch == '\n')
                        break;
                }
                temp[bytesRead+1] = '\0';
                DEBUG('e', "Se lee %s", temp);
                // machine->WriteRegister(2, -1);
                WriteStringToUser(temp, bufferPointer);
            }
            else{
                DEBUG('e', "`Read` requested from file with id %u.\n", id);
                // obtener file abiertos del hilo y leer los datos
                OpenFile *file = currentThread->Files->Get(id);

                if (file != nullptr) {
                    bytesRead = file->Read(temp, size);
                    temp[bytesRead] = '\0';
                    WriteStringToUser(temp, bufferPointer);
                    machine->WriteRegister(2, bytesRead);
                    break;
                }
                else {
                    DEBUG('e', "Error: could not open file with id %u for reading.\n",
                          id);
                    machine->WriteRegister(2, -1);
                    break;
                }
            }

            break;
        }


        case SC_OPEN: {
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0){
                DEBUG('e', "`Error`: address to filename string is null.\n");
                machine->WriteRegister(2, -1);
                break;
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr, filename, sizeof filename)) {
                DEBUG('e', "`Error`: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
                machine->WriteRegister(2, -1);
                break;
            }

            DEBUG('e', "`Open` requested for file `%s`.\n", filename);

            OpenFile *file = fileSystem->Open(filename);

            if(file == nullptr){
                DEBUG('e', "`Error`: could not open file `%s`.\n", filename);
                 machine->WriteRegister(2, -1);
                 break;
            }

            int idFile = currentThread->Files->Add(file);
            if(idFile == -1) {
                DEBUG('e', "`Error`:  files table is full.\n");
                delete file;
                machine->WriteRegister(2, -1);
                break;
            }
            DEBUG('e', "`Open` finished for file `%s`.\n", filename);
            machine->WriteRegister(2,idFile);
            break;
        }

        case SC_EXIT:{
            int status = machine->ReadRegister(4);
            DEBUG('e', "`Exit` requested with status %u.\n", status);
            // currentThread->Finish(status);
            currentThread->Finish();
            break;
        }

        case SC_CLOSE: {
            int fid = machine->ReadRegister(4);
            DEBUG('e', "`Close` requested for id %u.\n", fid);

            if (fid < 2) {
                DEBUG('e', "Error: file id must be greater than or equal to 2.\n");
                machine->WriteRegister(2, -1);
                break;
            }
            OpenFile *file = currentThread->Files->Remove(fid);
            if (file != nullptr) {
                delete file;
                machine->WriteRegister(2, 0);
            }
            else {
                DEBUG('e', "Error: could not close file with id %u.\n", fid);
                machine->WriteRegister(2, -1);
            }
            break;
        }

        case SC_PS: {
            DEBUG('e', "Print scheduler state\n");
            scheduler->Print();
            break;
        }

        default:
            fprintf(stderr, "Unexpected system call: id %d.\n", scid);
            ASSERT(false);

    }

    IncrementPC();
}


/// By default, only system calls have their own handler.  All other
/// exception types are assigned the default handler.
void
SetExceptionHandlers()
{
    machine->SetHandler(NO_EXCEPTION,            &DefaultHandler);
    machine->SetHandler(SYSCALL_EXCEPTION,       &SyscallHandler);
    machine->SetHandler(PAGE_FAULT_EXCEPTION,    &DefaultHandler);
    machine->SetHandler(READ_ONLY_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(BUS_ERROR_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(ADDRESS_ERROR_EXCEPTION, &DefaultHandler);
    machine->SetHandler(OVERFLOW_EXCEPTION,      &DefaultHandler);
    machine->SetHandler(ILLEGAL_INSTR_EXCEPTION, &DefaultHandler);
}
