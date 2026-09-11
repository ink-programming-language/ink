// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

class SA
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var S: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var r2: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var sa: dynamic = cpp_uninitialized();
  func SA() -> dynamic
  {
    }
  func SA(S: dynamic) -> dynamic
  {
      self->S = cpp_construct(S);
      init();
    }
  func init() -> dynamic
  {
      n = S.size();
      r.resize((n + 1), 0);
      r2.resize((n + 1), 0);
      t.resize((n + 1), 0);
      sa.resize((n + 1), 0);
      constract_sa();
    }
  func compare_sa(i: dynamic, j: dynamic) -> dynamic
  {
      if ((r[i] != r[j]))
      {
        return (r[i] < r[j]);
      } else
      {
        var ri: dynamic =  (((i + k) <= n)) ? r[(i + k)] : -1;
        var rj: dynamic =  (((j + k) <= n)) ? r[(j + k)] : -1;
        return (ri < rj);
      }
    }
  func constract_sa() -> dynamic
  {
      n = S.length();
      {
        var i: dynamic = 0;
        while ((i <= n))
        {
          sa[i] = i;
          r[i] =  ((i < n)) ? S[i] : -1;
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
            var i: dynamic = 1;
            while ((i <= n))
            {
              t[sa[i]] = (t[sa[(i - 1)]] + ( (compare_sa(sa[(i - 1)], sa[i])) ? 1 : 0));
              i += 1;
            }
          }
          {
            var i: dynamic = 0;
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
  func contains(T: dynamic) -> dynamic
  {
      var a: dynamic = 0;
      var b: dynamic = (S.length() + 1);
      while (((a + 1) < b))
      {
        var c: dynamic = (((a + b)) / 2);
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

var buf: dynamic = cpp_array(1000001);

func main() -> dynamic
{
  scanf("%s", buf);
  var q: dynamic = cpp_uninitialized();
  scanf("%lld", (&q));
  while (cpp_update(q, "--"))
  {
    scanf("%s", buf);
    printf("%lld\n", cpp_cast(sa.contains(P)));
  }
  return 0;
}

func __cpp_lambda_1(i: dynamic, j: dynamic) -> dynamic
{
  if ((r[i] != r[j]))
  {
    return (r[i] < r[j]);
  } else
  {
    var ri: dynamic =  (((i + k) <= n)) ? r[(i + k)] : -1;
    var rj: dynamic =  (((j + k) <= n)) ? r[(j + k)] : -1;
    return (ri < rj);
  }
}
