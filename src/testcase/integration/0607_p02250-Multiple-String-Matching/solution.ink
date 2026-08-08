// Translated from solution.cpp.

var int_cpp = dynamic;

class SA
{
  var n: dynamic;
  var k: dynamic;
  var S: dynamic;
  var r: dynamic;
  var r2: dynamic;
  var t: dynamic;
  var sa: dynamic;
  func SA()
  {
    }
  func SA(S: dynamic)
  {
      this->S = cpp_construct(S);
      init();
    }
  func init()
  {
      n = S.size();
      r.resize((n + 1), 0);
      r2.resize((n + 1), 0);
      t.resize((n + 1), 0);
      sa.resize((n + 1), 0);
      constract_sa();
    }
  func compare_sa(i: dynamic, j: dynamic)
  {
      if ((r[i] != r[j]))
      {
        return (r[i] < r[j]);
      } else
      {
        var ri = if (((i + k) <= n)) r[(i + k)] else -1;
        var rj = if (((j + k) <= n)) r[(j + k)] else -1;
        return (ri < rj);
      }
    }
  func constract_sa()
  {
      n = S.length();
      {
        var i = 0;
        while ((i <= n))
        {
          sa[i] = i;
          r[i] = if ((i < n)) S[i] else -1;
          i += 1;
        }
      }
      {
        k = 1;
        while ((k <= n))
        {
          sort(sa.begin(), sa.end(), __cpp_lambda_1);
          t[sa[0]] = 0;
          {
            var i = 1;
            while ((i <= n))
            {
              t[sa[i]] = (t[sa[(i - 1)]] + (if (compare_sa(sa[(i - 1)], sa[i])) 1 else 0));
              i += 1;
            }
          }
          {
            var i = 0;
            while ((i <= n))
            {
              r[i] = t[i];
              i += 1;
            }
          }
          k *= 2;
        }
      }
    }
  func contains(T: dynamic)
  {
      var a = 0;
      var b = (S.length() + 1);
      while (((a + 1) < b))
      {
        var c = (((a + b)) / 2);
        if ((S.compare(sa[c], T.length(), T) < 0))
        {
          a = c;
        } else
        {
          b = c;
        }
      }
      if ((b == (cpp_cast(S.length()) + 1)))
      {
        b -= 1;
      }
      return (S.compare(sa[b], T.length(), T) == 0);
    }
}

var buf = cpp_array(1000001);

func main()
{
  scanf("%s", buf);
  var q: dynamic;
  scanf("%lld", (&q));
  while (cpp_update(q, "--"))
  {
    scanf("%s", buf);
    printf("%lld\n", cpp_cast(sa.contains(P)));
  }
  return 0;
}

func __cpp_lambda_1(i: dynamic, j: dynamic)
{
  if ((r[i] != r[j]))
  {
    return (r[i] < r[j]);
  } else
  {
    var ri = if (((i + k) <= n)) r[(i + k)] else -1;
    var rj = if (((j + k) <= n)) r[(j + k)] else -1;
    return (ri < rj);
  }
}
