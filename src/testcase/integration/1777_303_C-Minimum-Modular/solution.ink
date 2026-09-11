// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var mmax: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(5005);

var f: dynamic = cpp_array(1000005);

var smax: dynamic = cpp_uninitialized();

var flag: dynamic = cpp_array(1000005);

func comp(a: dynamic, b: dynamic) -> dynamic
{
  return ((*cpp_cast(a)) - (*cpp_cast(b)));
}

func calcans(argument_0: dynamic) -> dynamic
{
  var ans: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var same: dynamic = cpp_uninitialized();
  {
    ans = 1;
    while (true)
    {
      {
        i = ans;
        same = 0;
        while ((i <= smax))
        {
          same += f[i];
          if ((same > mmax))
          {
            break;
          }
          i += ans;
        }
      }
      if ((same > mmax))
      {
        ans += 1;
        continue;
      }
      {
        i = cpp_assign(same, "=", 0);
        while ((i < n))
        {
          if ((!flag[(s[i] % ans)]))
          {
            flag[(s[i] % ans)] = true;
          } else
          {
            same += 1;
          }
          if ((same > m))
          {
            break;
          }
          i += 1;
        }
      }
      if ((same <= m))
      {
        return ans;
      }
      {
        j = 0;
        while ((j < i))
        {
          flag[(s[j] % ans)] = false;
          j += 1;
        }
      }
      ans += 1;
    }
  }
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&m));
  mmax = ((((m + 1)) * m) / 2);
  memset(f, 0, cpp_sizeof((f)));
  memset(flag, 0, cpp_sizeof((flag)));
  {
    i = 0;
    while ((i < n))
    {
      scanf("%d", (&s[i]));
      i += 1;
    }
  }
  qsort(s, n, cpp_sizeof(dynamic), comp);
  smax = (s[(n - 1)] - s[0]);
  {
    i = 0;
    while ((i < n))
    {
      {
        j = (i + 1);
        while ((j < n))
        {
          f[(s[j] - s[i])] += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", calcans());
  return 0;
}
