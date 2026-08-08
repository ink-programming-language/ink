// Translated from solution.cpp.

var a = cpp_array(300005);

func main()
{
  scanf("%s", (a + 1));
  var n = strlen((a + 1));
  var ans = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      var nl = min((i + 12), (n + 1));
      ans += (((n + 1) - nl));
      var fl = 0;
      {
        var j = i;
        while ((j < nl))
        {
          var fl = 0;
          {
            var k = i;
            while ((k <= j))
            {
              if (fl)
              {
                break;
              }
              {
                var s = 1;
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
