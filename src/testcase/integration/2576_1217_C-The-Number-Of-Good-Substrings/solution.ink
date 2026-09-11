// Translated from solution.cpp.

var maxn: dynamic = (2e5 + 100);

var inf: dynamic = 0x3f3f3f3f;

var s: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  scanf("%d", (&t));
  getchar();
  while (cpp_update(t, "--"))
  {
    scanf("%s", (s + 1));
    var ans: dynamic = 0;
    var last: dynamic = 0;
    var len: dynamic = strlen((s + 1));
    {
      var i: dynamic = 1;
      while ((i <= len))
      {
        if ((s[i] == cpp_char("1")))
        {
          var k: dynamic = 0;
          var l: dynamic = 0;
          {
            var j: dynamic = i;
            while (((j <= len) && (j < (i + 20))))
            {
              k = (((k * 2) + s[j]) - cpp_char("0"));
              l = ((j - k) + 1);
              if ((l <= last))
              {
                break;
              }
              ans += 1;
              j += 1;
            }
          }
          last = i;
        }
        i += 1;
      }
    }
    printf("%lld\n", ans);
  }
}
