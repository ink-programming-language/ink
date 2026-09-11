// Translated from solution.cpp.

var inf: dynamic = (1e18 + 7);

var N: dynamic = 500007;

var ar: dynamic = cpp_array(N);

var fr: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var sum: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var mx: dynamic = cpp_uninitialized();

var mn: dynamic = inf;

var s: dynamic = cpp_array(55);

var v: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  {
    i = 1;
    while ((i <= n))
    {
      read(ar[i]);
      fr[ar[i]] += 1;
      i += 1;
    }
  }
  i = 4;
  if ((n == 1))
  {
    write(ar[1], "\n");
  } else if ((n == 2))
  {
    if ((fr[1] == 1))
    {
      write(2, cpp_char(" "), 1, "\n");
    } else
    {
      write(ar[1], cpp_char(" "), ar[2], "\n");
    }
  } else if ((n == 3))
  {
    if ((fr[1] == 1))
    {
      write(2, cpp_char(" "), 1, cpp_char(" "), 2, "\n");
    } else if ((fr[1] == 2))
    {
      write(2, cpp_char(" "), 1, cpp_char(" "), 1, "\n");
    } else
    {
      write(ar[1], cpp_char(" "), ar[2], cpp_char(" "), ar[3], "\n");
    }
  } else if ((fr[1] > 2))
  {
    write(1, cpp_char(" "), 1, cpp_char(" "), 1);
    fr[1] -= 3;
  } else
  {
    write(2);
    fr[2] -= 1;
    if (fr[1])
    {
      write(cpp_char(" "), 1);
      i = 3;
      fr[1] -= 1;
    } else
    {
      i = 2;
    }
  }
  {
    while ((i <= n))
    {
      if (fr[2])
      {
        write(cpp_char(" "), 2);
        fr[2] -= 1;
      } else if ((fr[1] > 1))
      {
        write(cpp_char(" "), 1, cpp_char(" "), 1);
        fr[1] -= 2;
        i += 1;
      } else
      {
        write(cpp_char(" "), 1);
      }
      i += 1;
    }
  }
}
