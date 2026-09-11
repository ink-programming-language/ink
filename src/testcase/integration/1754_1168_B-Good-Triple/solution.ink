// Translated from solution.cpp.

var a: dynamic = cpp_array(300005);

func main() -> dynamic
{
  scanf("%s", (a + 1));
  var n: dynamic = strlen((a + 1));
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var nl: dynamic = min((i + 12), (n + 1));
      ans += (((n + 1) - nl));
      var fl: dynamic = 0;
      {
        var j: dynamic = i;
        while ((j < nl))
        {
          var fl: dynamic = 0;
          {
            var k: dynamic = i;
            while ((k <= j))
            {
              if (fl)
              {
                break;
              }
              {
                var s: dynamic = 1;
                while ((s <= min((k - i), (j - k))))
                {
                  if (((a[k] == a[(k - s)]) && (a[k] == a[(k + s)])))
                  {
                    fl = 1;
                    break;
                  }
                  s += 1;
                }
              }
              k += 1;
            }
          }
          ans += fl;
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
