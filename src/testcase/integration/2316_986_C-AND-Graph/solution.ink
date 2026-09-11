// Translated from solution.cpp.

var MAXN: dynamic = (1 << (22 + 1));

var pai: dynamic = cpp_array(MAXN);

var ranki: dynamic = cpp_array(MAXN);

var tmp: dynamic = cpp_array(MAXN);

var pd: dynamic = cpp_array(MAXN);

var inp: dynamic = cpp_array(MAXN);

class ConjDisj
{
  var n: dynamic = cpp_uninitialized();
  func ConjDisj(N: dynamic) -> dynamic
  {
      {
        var i: dynamic = 0;
        var n: dynamic = N;
        while ((i < n))
        {
          pai[i] = i;
          ranki[i] = 0;
          i += 1;
        }
      }
    }
  func busca(x: dynamic) -> dynamic
  {
      if ((x != pai[x]))
      {
        pai[x] = busca(pai[x]);
      }
      return pai[x];
    }
  func uniao(a: dynamic, b: dynamic) -> dynamic
  {
      if (((!inp[a]) || (!inp[b])))
      {
        return;
      }
      var paiA: dynamic = busca(a);
      var paiB: dynamic = busca(b);
      if ((ranki[paiA] < ranki[paiB]))
      {
        pai[paiA] = paiB;
      } else
      {
        if ((ranki[paiA] == ranki[paiB]))
        {
          ranki[paiA] += 1;
        }
        pai[paiB] = paiA;
      }
    }
}

var cd: dynamic = cpp_construct((MAXN - 1));

func solve(mask: dynamic, W: dynamic) -> dynamic
{
  if ((!pd[mask]))
  {
    {
      var i: dynamic = (1 << 21);
      while ((i > 0))
      {
        if ((i & mask))
        {
          var nmask: dynamic = (mask ^ i);
          cd.uniao(W, nmask);
          solve(nmask, W);
        }
        i >>= 1;
      }
    }
    pd[mask] = true;
  }
  return pd[mask];
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  scanf(" %d %d", (&n), (&m));
  memset(pd, 0, cpp_sizeof(pd));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var x: dynamic = cpp_uninitialized();
      scanf(" %d", (&x));
      inp[x] = true;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (1 << 22)))
    {
      if ((!inp[i]))
      {
        i += 1;
        continue;
      }
      var x: dynamic = i;
      var w: dynamic = (((((1 << 22)) - 1)) ^ x);
      cd.uniao(x, w);
      solve(w, x);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (1 << 22)))
    {
      if (inp[i])
      {
        tmp[cd.busca(i)] = 1;
      }
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (1 << 22)))
    {
      if (tmp[i])
      {
        ans += 1;
      }
      i += 1;
    }
  }
  printf("%d\n", ans);
}
