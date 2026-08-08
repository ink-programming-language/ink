// Translated from solution.cpp.

var maxn = (2e5 + 100);

var inf = 0x3f3f3f3f;

var s = cpp_array(maxn);

func main()
{
  var t: dynamic;
  scanf("%d", (&t));
  getchar();
  while (cpp_update(t, "--"))
  {
    scanf("%s", (s + 1));
    var ans = 0;
    var last = 0;
    var len = strlen((s + 1));
    {
      var i = 1;
      while ((i <= len))
      {
        if ((s[i] == cpp_char("1")))
        {
          var k = 0;
          var l = 0;
          {
            var j = i;
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
