// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(200005);

var t: dynamic = cpp_array(200005);

var ss: dynamic = cpp_array(200005);

func main() -> dynamic
{
  scanf("%s", ss);
  var n: dynamic = strlen(ss);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      s[i] = (ss[(i - 1)] - cpp_char("0"));
      i += 1;
    }
  }
  var tot: dynamic = 0;
  {
    var i: dynamic = n;
    while ((i >= 1))
    {
      if ((tot && s[i]))
      {
        t[i] = 1;
      } else
      {
        t[i] = 0;
      }
      if ((s[i] == 0))
      {
        tot += 1;
      }
      if ((s[i] == 1))
      {
        tot -= 1;
      }
      tot = max(tot, 0);
      i -= 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      printf("%d", t[i]);
      i += 1;
    }
  }
  return 0;
}
