// Translated from solution.cpp.

var n: dynamic;

var a: dynamic;

var t: dynamic;

var sum: dynamic;

func output(x: dynamic)
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  var len = 0;
  var data = cpp_array(10);
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

func main()
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
