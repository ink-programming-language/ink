// Translated from solution.cpp.

func abs(a: dynamic) -> dynamic
{
  return ( (((a < 0))) ? (-a) : a);
}

func sqr(a: dynamic) -> dynamic
{
  return (a * a);
}

var INF: dynamic = (cpp_cast(1E9) + 7);

var EPS: dynamic = 1E-9;

var PI: dynamic = 3.1415926535897932384626433832795;

var s: dynamic = cpp_array(2000000);

func main() -> dynamic
{
  var n: dynamic = strlen(gets(s));
  var ans: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < int_cpp(n)))
    {
      if ((s[i] == cpp_char("0")))
      {
        i += 1;
        continue;
      }
      if (((s[i] == cpp_char("1")) && ((((i + 1) >= n) || (s[(i + 1)] == cpp_char("0"))))))
      {
        ans.push_back(make_pair(1, i));
        i += 1;
        continue;
      }
      var j: dynamic = i;
      while ((j < n))
      {
        while (((j < n) && (s[i] == s[j])))
        {
          j += 1;
        }
        if ((((j + 1) < n) && (s[(j + 1)] == cpp_char("1"))))
        {
          ans.push_back(make_pair(-1, j));
          j += 1;
        } else
        {
          break;
        }
      }
      ans.push_back(make_pair(1, (i - 1)));
      ans.push_back(make_pair(-1, (j - 1)));
      i = (j - 1);
      i += 1;
    }
  }
  write(int_cpp((ans).size()), "\n");
  {
    var i: dynamic = 0;
    while ((i < int_cpp(int_cpp((ans).size()))))
    {
      if ((ans[i].first == 1))
      {
        printf("+2^%d\n", ((n - ans[i].second) - 1));
      } else
      {
        printf("-2^%d\n", ((n - ans[i].second) - 1));
      }
      i += 1;
    }
  }
  return 0;
}
