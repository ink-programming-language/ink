// Translated from solution.cpp.

var MOD: dynamic = 1000000007;

var EPS: dynamic = 1e-8;

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var K: dynamic = cpp_uninitialized();

var H: dynamic = cpp_uninitialized();

var W: dynamic = cpp_uninitialized();

var L: dynamic = cpp_uninitialized();

var R: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var s: dynamic = cpp_uninitialized();
  read(s);
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 3))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < (i + 4)))
        {
          if ((j >= s.size()))
          {
            break;
          }
          {
            var k: dynamic = (j + 1);
            while ((k < (j + 4)))
            {
              if ((k >= s.size()))
              {
                break;
              }
              {
                var l: dynamic = (k + 1);
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
                  var a: dynamic = 0;
                  var b: dynamic = 0;
                  var c: dynamic = 0;
                  var d: dynamic = 0;
                  {
                    var m: dynamic = 0;
                    while ((m <= i))
                    {
                      a *= 10;
                      a += (s[m] - cpp_char("0"));
                      m += 1;
                    }
                  }
                  {
                    var m: dynamic = (i + 1);
                    while ((m <= j))
                    {
                      b *= 10;
                      b += (s[m] - cpp_char("0"));
                      m += 1;
                    }
                  }
                  {
                    var m: dynamic = (j + 1);
                    while ((m <= k))
                    {
                      c *= 10;
                      c += (s[m] - cpp_char("0"));
                      m += 1;
                    }
                  }
                  {
                    var m: dynamic = (k + 1);
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
