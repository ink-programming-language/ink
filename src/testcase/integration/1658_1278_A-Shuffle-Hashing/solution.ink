// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return (((a * b)) / gcd(a, b));
}

func sortbysec(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.second < b.second));
}

func main() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var flag: dynamic = cpp_uninitialized();
  read(x);
  var s: dynamic = cpp_uninitialized();
  var s1: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var v1: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < x))
    {
      read(s);
      read(s1);
      if ((s.size() <= s1.size()))
      {
        {
          j = 0;
          while ((j < s.size()))
          {
            v.push_back(s[j]);
            j += 1;
          }
        }
        sort(v.begin(), v.end());
        {
          j = 0;
          while ((j <= (s1.size() - s.size())))
          {
            {
              k = j;
              while ((k < (j + s.size())))
              {
                v1.push_back(s1[k]);
                k += 1;
              }
            }
            flag = 0;
            sort(v1.begin(), v1.end());
            {
              k = 0;
              while ((k < v1.size()))
              {
                if ((v1[k] != v[k]))
                {
                  flag = 1;
                  break;
                }
                k += 1;
              }
            }
            if ((flag == 0))
            {
              break;
            }
            v1.clear();
            j += 1;
          }
        }
        v.clear();
        v1.clear();
        if ((flag == 0))
        {
          write("YES", "\n");
        } else
        {
          write("NO", "\n");
        }
      } else
      {
        write("NO", "\n");
      }
      v.clear();
      v1.clear();
      i += 1;
    }
  }
}
