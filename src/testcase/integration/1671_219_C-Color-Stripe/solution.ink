// Translated from solution.cpp.

var MAX = (5e5 + 9);

var s = cpp_array(MAX);

var a = cpp_array(MAX);

var b = cpp_array(MAX);

func main()
{
  var n: dynamic;
  var k: dynamic;
  var ans = 0;
  var c1: dynamic;
  var c2: dynamic;
  scanf("%d%d%s", (&n), (&k), (&s));
  if ((k == 2))
  {
    c1 = cpp_assign(c2, "=", 0);
    {
      var i = 0;
      while ((i < n))
      {
        if ((i % 2))
        {
          a[i] = cpp_char("A");
          c1 += ((s[i] == cpp_char("B")));
          b[i] = cpp_char("B");
          c2 += ((s[i] == cpp_char("A")));
        } else
        {
          a[i] = cpp_char("B");
          c1 += ((s[i] == cpp_char("A")));
          b[i] = cpp_char("A");
          c2 += ((s[i] == cpp_char("B")));
        }
        i += 1;
      }
    }
    return (!printf("%d\n%s", min(c1, c2), if ((c1 < c2)) a else b));
  }
  {
    var i = 1;
    while (s[i])
    {
      if ((s[i] == s[(i - 1)]))
      {
        {
          var j = 0;
          while ((j < k))
          {
            if (((s[(i - 1)] != ((j + cpp_char("A")))) && (s[(i + 1)] != ((j + cpp_char("A"))))))
            {
              s[i] = (j + cpp_char("A"));
              break;
            }
            j += 1;
          }
        }
        ans += 1;
      }
      i += 1;
    }
  }
  printf("%d\n%s", ans, s);
}
