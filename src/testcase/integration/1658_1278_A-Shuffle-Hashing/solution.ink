// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func lcm(a: dynamic, b: dynamic)
{
  return (((a * b)) / gcd(a, b));
}

func sortbysec(a: dynamic, b: dynamic)
{
  return ((a.second < b.second));
}

func main()
{
  var x: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var flag: dynamic;
  read(x);
  var s: dynamic;
  var s1: dynamic;
  var v: dynamic;
  var v1: dynamic;
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
