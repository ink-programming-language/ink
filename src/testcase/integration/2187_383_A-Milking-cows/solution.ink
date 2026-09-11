// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var sum: dynamic = cpp_uninitialized();

func output(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  var len: dynamic = 0;
  var data: dynamic = cpp_array(10);
  while (x)
  {
    data[cpp_update(len, "++")] = (x % 10);
    x /= 10;
  }
  if ((!len))
  {
    data[cpp_update(len, "++")] = 0;
  }
  while (cpp_update(len, "--"))
  {
    putchar((data[len] + 48));
  }
  putchar(cpp_char("\n"));
}

func main() -> dynamic
{
  read(n);
  while (cpp_update(n, "--"))
  {
    read(a);
    if (a)
    {
      t += a;
    } else
    {
      sum += t;
    }
  }
  output(sum);
}
