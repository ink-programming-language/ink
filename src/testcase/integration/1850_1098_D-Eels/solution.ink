// Translated from solution.cpp.

var sum = cpp_array(101);

var s = cpp_array(101);

func main()
{
  var Q: dynamic;
  scanf("%d", (&Q));
  while (cpp_update(Q, "--"))
  {
    var c = cpp_array(10);
    var x: dynamic;
    scanf("%s%d", c, (&x));
    var i = 0;
    {
      while ((((1 << ((i + 1)))) <= x))
      {
        i += 1;
      }
    }
    if ((c[0] == cpp_char("+")))
    {
      sum[i] += x;
      s[i].insert(x);
    } else
    {
      sum[i] -= x;
      s[i].erase(s[i].find(x));
    }
    var S = 0;
    var ans = 0;
    {
      i = 0;
      while ((i <= 30))
      {
        ans += s[i].size();
        if (((!s[i].empty()) && ((*s[i].begin()) > (2 * S))))
        {
          ans -= 1;
        }
        S += sum[i];
        i += 1;
      }
    }
    printf("%d\n", ans);
  }
}
