// Translated from solution.cpp.

var Left = cpp_array(101000);

var Right = cpp_array(101000);

var x = cpp_array(101000);

var t = cpp_array(101000);

var n: dynamic;

var V: dynamic;

var s = cpp_array(101000);

var p = cpp_array(101000);

var f = cpp_array(101000);

var L: dynamic;

var R: dynamic;

var MAX: dynamic;

var Ans: dynamic;

var Ans2: dynamic;

func cmp(a: dynamic, b: dynamic)
{
  return if ((Left[a] == Left[b])) (t[a] < t[b]) else (Left[a] < Left[b]);
}

func main()
{
  var i: dynamic;
  scanf("%d", (&n));
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d%d", (x + i), (t + i));
      if ((t[i] > MAX))
      {
        MAX = t[i];
      }
      p[i] = cpp_update(i, "++");
    }
  }
  n += 1;
  scanf("%d", (&V));
  {
    i = 0;
    while ((i < n))
    {
      Left[i] = (x[i] - ((1 * ((MAX - t[i]))) * V));
      Right[i] = (x[i] + ((1 * ((MAX - t[i]))) * V));
      i += 1;
    }
  }
  sort(p, (p + n), cmp);
  {
    i = (n - 1);
    while ((i >= 0))
    {
      L = 0;
      R = (Ans + 1);
      {
        while (((L + 1) < R))
        {
          if ((Right[s[(((L + R)) / 2)]] <= Right[p[i]]))
          {
            L = (((L + R)) / 2);
          } else
          {
            R = (((L + R)) / 2);
          }
        }
      }
      if (((!Ans) || (Right[s[L]] > Right[p[i]])))
      {
        f[p[i]] = 1;
      } else
      {
        f[p[i]] = (f[s[L]] + 1);
      }
      if ((f[p[i]] > Ans))
      {
        Ans = f[cpp_assign(s[f[p[i]]], "=", p[i])];
      }
      if ((p[i] && (f[p[i]] > Ans2)))
      {
        Ans2 = f[p[i]];
      }
      if ((Right[s[f[p[i]]]] > Right[p[i]]))
      {
        s[f[p[i]]] = p[i];
      }
      i -= 1;
    }
  }
  printf("%d %d\n", (f[0] - 1), Ans2);
  return 0;
}
