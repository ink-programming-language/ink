// Translated from solution.cpp.

func main()
{
  var t: dynamic;
  read(t);
  var n: dynamic;
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var s: dynamic;
    read(n);
    read(s);
    var flag = 1;
    var start = 0;
    var end = (n - 1);
    while ((start <= end))
    {
      var ssp = (s[start] + 1);
      var sep = (s[end] + 1);
      var sem = (s[end] - 1);
      var ssm = (s[start] - 1);
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
