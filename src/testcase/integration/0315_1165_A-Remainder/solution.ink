// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var sum: dynamic = 0;
  var str: dynamic = cpp_uninitialized();
  read(n, x, y, str);
  var len: dynamic = (str.size() - 1);
  {
    var i: dynamic = len;
    while ((i > (len - x)))
    {
      if ((str[i] == cpp_char("1")))
      {
        sum += 1;
      }
      i -= 1;
    }
  }
  if ((str[(len - y)] == cpp_char("0")))
  {
    sum += 1;
  } else
  {
    sum -= 1;
  }
  write(sum, "\n");
  return 0;
}
