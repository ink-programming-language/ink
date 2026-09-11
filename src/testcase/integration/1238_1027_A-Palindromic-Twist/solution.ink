// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  var n: dynamic = cpp_uninitialized();
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var s: dynamic = cpp_uninitialized();
    read(n);
    read(s);
    var flag: dynamic = 1;
    var start: dynamic = 0;
    var end: dynamic = (n - 1);
    while ((start <= end))
    {
      var ssp: dynamic = (s[start] + 1);
      var sep: dynamic = (s[end] + 1);
      var sem: dynamic = (s[end] - 1);
      var ssm: dynamic = (s[start] - 1);
      if ((((((ssp == sep)) || ((ssp == sem))) || ((ssm == sep))) || ((ssm == sem))))
      {
        flag = 1;
      } else
      {
        flag = 0;
        break;
      }
      start += 1;
      end -= 1;
    }
    if ((flag == 0))
    {
      write("NO", "\n");
    } else
    {
      write("YES", "\n");
    }
  }
  return 0;
}
