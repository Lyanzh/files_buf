# GNU C对ANSI C的9条扩展语法 #

摘取自《Linux设备驱动开发详解》第二版·宋宝华

Linux 上可用的 C 编译器是 GNU C 编译器，它建立在自由软件基金会的编程许可证的基础上，因此可以自由发布。 GNU C 对标准 C 进行一系列扩展，以增强标准 C 的功能。

## 1． 零长度和变量长度数组 ##

GNU C 允许使用零长度数组，在定义变长对象的头结构时，这个特性非常有用。例如：

    struct var_data {
    	int len;
    	char data[0];
    };

char data[0]仅仅意味着程序中通过 var_data 结构体实例的 data[index]成员可以访问 len 之后的第 index 个地址，它并没有为 data[]数组分配内存，因此 sizeof(struct var_data)=sizeof(int)。

假设 struct var_data 的数据域就保存在 struct var_data 紧接着的内存区域，则通过如下代码可以遍历这些数据：

    struct var_data s;
    ...
    for (i = 0; i < s.len; i++)
    	printf("%02x", s.data[i]);

GNU C 中也可以使用变量定义数组，例如如下代码中定义的“double x[n]”：

    int main (int argc, char *argv[])
    {
    	int i, n = argc;
    	double x[n];
    	for (i = 0; i < n; i++)
    		x[i] = i;
    	return 0;
    }

## 2． case 范围 ##

GNU C 支持 case x…y 这样的语法，区间[x,y]的数都会满足这个 case 的条件，请看下面的代码：

    switch (ch) {
    case '0'... '9': c -= '0';
    	break;
    case 'a'... 'f': c -= 'a' - 10;
    	break;
    case 'A'... 'F': c -= 'A' - 10;
    	break;
    }

代码中的 case '0' ... '9'等价于标准 C 中的：

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':

## 3． 语句表达式 ##

GNU C 把包含在括号中的复合语句看做是一个表达式，称为语句表达式，它可以出现在任何允许表达式的地方。我们可以在语句表达式中使用原本只能在复合语句中使用的循环、局部变量等，例如：

    #define min_t(type,x,y) \
    ({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })
    int ia, ib, mini;
    float fa, fb, minf;
    mini = min_t(int, ia, ib);
    minf = min_t(float, fa, fb);

因为重新定义了 __xx 和 __y 这两个局部变量，所以以上述方式定义的宏将不会有副作用。在标准 C 中，对应的如下宏则会产生副作用：

    #define min(x,y) ((x) < (y) ? (x) : (y))

代码 min(++ia,++ib)会被展开为((++ia) < (++ib) ? (++ia): (++ib))，传入宏的“参数” 被增加2 次。

## 4． typeof 关键字 ##

typeof(x)语句可以获得 x 的类型，因此，我们可以借助 typeof 重新定义 min 这个宏：

    #define min(x,y) ({ \
    const typeof(x) _x = (x); \
    const typeof(y) _y = (y); \
    (void) (&_x == &_y); \
    _x < _y ? _x : _y; })

我们不需要像 min_t(type,x,y)这个宏那样把 type 传入，因为通过 typeof(x)、 typeof(y)可以获得type。代码行(void) (& _x == & _y) 的作用是检查 _x 和 _y 的类型是否一致。

## 5． 可变参数宏 ##

标准 C 就支持可变参数函数，意味着函数的参数是不固定的，例如 printf()函数的原型为：

    int printf( const char *format [, argument]... );

而在 GNU C 中，宏也可以接受可变数目的参数，例如：

    #define pr_debug(fmt,arg...) \
    			printk(fmt,##arg)

这里 arg 表示其余的参数，可以是零个或多个，这些参数以及参数之间的逗号构成 arg 的值，在宏扩展时替换 arg，例如下列代码：

    pr_debug("%s:%d",filename,line)

会被扩展为：

    printk("%s:%d", filename, line)

使用“##” 的原因是处理 arg 不代表任何参数的情况，这时候，前面的逗号就变得多余了。

使用“##” 之后， GNU C 预处理器会丢弃前面的逗号，这样，代码：

    pr_debug("success!\n")

会被正确地扩展为：

    printk("success!\n")

而不是：

    printk("success!\n",)

这正是我们希望看到的。

## 6． 标号元素 ##

标准 C 要求数组或结构体的初始化值必须以固定的顺序出现，在 GNU C 中，通过指定索引或结构体成员名，允许初始化值以任意顺序出现。

指定数组索引的方法是在初始化值前添加“[INDEX] =”，当然也可用“[FIRST ... LAST] =”的形式指定一个范围。例如，下面的代码定义一个数组，并把其中的所有元素赋值为 0：

    unsigned char data[MAX] = { [0 ... MAX-1] = 0 };

下面的代码借助结构体成员名初始化结构体：

    struct file_operations ext2_file_operations = {
	    llseek: generic_file_llseek,
	    read: generic_file_read,
	    write: generic_file_write,
	    ioctl: ext2_ioctl,
	    mmap: generic_file_mmap,
	    open: generic_file_open,
	    release: ext2_release_file,
	    fsync: ext2_sync_file,
    };

但是， Linux 2.6 推荐类似的代码应该尽量采用标准 C 的方式：

    struct file_operations ext2_file_operations = {
	    .llseek = generic_file_llseek,
	    .read = generic_file_read,
	    .write = generic_file_write,
	    .aio_read = generic_file_aio_read,
	    .aio_write = generic_file_aio_write,
	    .ioctl = ext2_ioctl,
	    .mmap = generic_file_mmap,
	    .open = generic_file_open,
	    .release= ext2_release_file,
	    .fsync = ext2_sync_file,
	    .readv = generic_file_readv,
	    .writev = generic_file_writev,
	    .sendfile = generic_file_sendfile,
    };

## 7． 当前函数名 ##

GNU C 预定义了两个标志符保存当前函数的名字， __ FUNCTION__ 保存函数在源码中的名字， __ PRETTY_FUNCTION__ 保存带语言特色的名字。在 C 函数中，这两个名字是相同的。

	void example()
	{
		printf("This is function:%s", __FUNCTION__);
	}

代码中的 __ FUNCTION__ 意味着字符串“example”。 C99 已经支持 __ func__ 宏，因此建议在Linux 编程中不再使用 __ FUNCTION__ ，而转而使用 __ func__：

	void example()
	{
		printf("This is function:%s", __func__);
	}

## 8． 特殊属性声明 ##

GNU C 允许声明函数、变量和类型的特殊属性，以便进行手工的代码优化和定制代码检查的方法。要指定一个声明的属性，只需要在声明后添加 __ attribute__ (( ATTRIBUTE ))。 其中ATTRIBUTE 为属性说明，如果存在多个属性，则以逗号分隔。 GNU C 支持 noreturn、 format、 section、
aligned、 packed 等十多个属性。

noreturn 属性作用于函数，表示该函数从不返回。这会让编译器优化代码，并消除不必要的警告信息。例如：

	# define ATTRIB_NORET __attribute__((noreturn)) ....
	asmlinkage NORET_TYPE void do_exit(long error_code) ATTRIB_NORET;

format 属性也用于函数，表示该函数使用 printf、 scanf 或 strftime 风格的参数，指定 format 属性可以让编译器根据格式串检查参数类型。例如：

	asmlinkage int printk(const char * fmt, ...) __attribute__ ((format (printf, 1, 2)));

上述代码中的第 1 个参数是格式串，从第 2 个参数开始都会根据 printf()函数的格式串规则检查参数。

unused 属性作用于函数和变量，表示该函数或变量可能不会被用到，这个属性可以避免编译
器产生警告信息。

aligned 属性用于变量、结构体或联合体，指定变量、结构体或联合体的对界方式，以字节为单位，例如：

	struct example_struct {
		char a;
		int b;
		long c;
	} _ _attribute_ _((aligned(4)));

表示该结构类型的变量以 4 字节对界。

## 9． 内建函数 ##

GNU C 提供了大量的内建函数，其中大部分是标准 C 库函数的 GNU C 编译器内建版本，例
如 memcpy()等，它们与对应的标准 C 库函数功能相同。

不属于库函数的其他内建函数的命名通常以 __builtin 开始， 如下所示。

- 内建函数 __ builtin_return_address (LEVEL) 返回当前函数或其调用者的返回地址，参数 LEVEL 指定调用栈的级数，如 0 表示当前函数的返回地址， 1 表示当前函数的调用者的返回地址。

- 内建函数 __ builtin_constant_p(EXP) 用于判断一个值是否为编译时常数，如果参数 EXP 的值是常数，函数返回 1，否则返回 0。

- 内建函数 __ builtin_expect(EXP, C) 用于为编译器提供分支预测信息，其返回值是整数表达式 EXP 的值， C 的值必须是编译时常数。

例如，下面的代码检测第 1 个参数是否为编译时常数以确定采用参数版本还是非参数版本的代码：

    #define test_bit(nr,addr) \
    (_ _builtin_constant_p(nr) ? \
    constant_test_bit((nr),(addr)) : \
    variable_test_bit((nr),(addr)))


----------

在使用 gcc 编译 C 程序的时候，如果使用“-ansi –pedantic”编译选项，则会告诉编译器不使
用 GNU 扩展语法。例如对于如下 C 程序 test.c：

	struct var_data {
		int len;
		char data[0];
	};
	struct var_data a;

直接编译可以通过：

	gcc -c test.c

如果使用“-ansi –pedantic”编译选项，编译会报警：

	gcc -ansi -pedantic -c test.c
	test.c:3: warning: ISO C forbids zero-size array ‘data’