// Translated from solution.cpp.

var MOD = 1000000007;

var EPS = 1e-8;

var N: dynamic;

var M: dynamic;

var K: dynamic;

var H: dynamic;

var W: dynamic;

var L: dynamic;

var R: dynamic;

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var s: dynamic;
  read(s);
  var ans = 0;
  {
    var i = 0;
    while ((i < 3))
    {
      {
        var j = (i + 1);
        while ((j < (i + 4)))
        {
          if ((j >= s.size()))
          {
            break;
          }
          {
            var k = (j + 1);
            while ((k < (j + 4)))
            {
              if ((k >= s.size()))
              {
                break;
              }
              {
                var l = (k + 1);
                while ((l < (k + 4)))
                {
                  if (((l + 1) != s.size()))
                  {
                    if ((s[(k + 1)] == cpp_char("0")))
                    {
                      break;
                    }
                    l += 1;
                    continue;
                  }
                  if ((l >= s.size()))
                  {
                    break;
                  }
                  var a = 0;
                  var b = 0;
                  var c = 0;
                  var d = 0;
                  {
                    var m = 0;
                    while ((m <= i))
                    {
                      a *= 10;
                      a += (s[m] - cpp_char("0"));
                      m += 1;
                    }
                  }
                  {
                    var m = (i + 1);
                    while ((m <= j))
                    {
                      b *= 10;
                      b += (s[m] - cpp_char("0"));
                      m += 1;
                    }
                  }
                  {
                    var m = (j + 1);
                    while ((m <= k))
                    {
                      c *= 10;
                      c += (s[m] - cpp_char("0"));
                      m += 1;
                    }
                  }
                  {
                    var m = (k + 1);
                    while ((m <= l))
                    {
                      d *= 10;
                      d += (s[m] - cpp_char("0"));
                      m += 1;
                    }
                  }
                  if (((((a <= 255) && (b <= 255)) && (c <= 255)) && (d <= 255)))
                  {
                    ans += 1;
                  }
                  if ((s[(k + 1)] == cpp_char("0")))
                  {
                    break;
                  }
                  l += 1;
                }
              }
              if ((s[(j + 1)] == cpp_char("0")))
              {
                break;
              }
              k += 1;
            }
          }
          if ((s[(i + 1)] == cpp_char("0")))
          {
            break;
          }
          j += 1;
        }
      }
      if ((s[0] == cpp_char("0")))
      {
        break;
      }
      i += 1;
    }
  }
  write(ans, "\n");
}
