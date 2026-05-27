__attribute__((weak)) int myargc;
__attribute__((weak)) char **myargv;

__attribute__((weak)) void D_DoomMain(void)
{
}


__attribute__((weak)) int IE_FileReadAll(const char *name, void *buffer,
                                        unsigned int buffer_len,
                                        unsigned int *result_len)
{
    (void) name;
    (void) buffer;
    (void) buffer_len;
    if (result_len != 0)
    {
        *result_len = 0;
    }
    return 0;
}

__attribute__((weak)) void M_FindResponseFile(void)
{
}

__attribute__((weak)) void M_SetExeDir(void)
{
}
