## 获取程序代码段的方法

- task_struct的成员变量mm是进程所拥有的内存空间描述符

```
struct task_struct{
...
struct mm_struct *mm, *active_mm;
...
}
```
- mm_struct的start_code为代码段起始地址
```
struct mm_struct{
...
    //维护代码段和数据段
    unsigned long start_code, end_code, start_data, end_data;
    //维护堆和栈
    unsigned long start_brk, brk, start_stack;
...

}
```
但要注意，这里只是代码的elf头的起始地址。可以把elf头理解为一种文件格式。里面记录了代码的文件类型，程序入口地址等信息.下面是他的数据结构，我只是通过e_entry成员变量找到了真正程序的入口地址。

- elf头数据结构中的e_entry成员变量是真正程序的入口地址。
```
typedef struct{
　　unsigned char e_ident[EI_NIDENT];
　　Elf32_Half e_type;
　　Elf32_Half e_machine;
　　Elf32_Word e_version;
　　Elf32_Addr e_entry;
　　Elf32_Off e_phoff;
　　Elf32_Off e_shoff;
　　Elf32_Word e_flags;
　　Elf32_Half e_ehsize;
　　Elf32_Half e_phentsize;
　　Elf32_Half e_phnum;
　　Elf32_Half e_shentsize;
　　Elf32_Half e_shnum;
　　Elf32_Half e_shstrndx;
　　}Elf32_Ehdr;
```
（程序并不是从main函数开始运行的，因此e_entry指向的地址，并不是main函数的地址，如果要得到main的地址，需要再从e_entry指向的地址进行分析，这里我没有找到对应的数据结构，我是分析从e_entry开始的后面的汇编代码，看见一条跳转指令，发现跳转的地址刚好是main函数地址，然后计算这条指令和e_entry指向地址的偏移量，发现，所有程序偏移量相同，就这样，我再e_entry的值上加一个固定偏移，就是main函数地址）
