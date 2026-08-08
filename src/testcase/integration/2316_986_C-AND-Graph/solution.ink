// Translated from solution.cpp.

var MAXN = (1 << (22 + 1));

var pai = cpp_array(MAXN);

var ranki = cpp_array(MAXN);

var tmp = cpp_array(MAXN);

var pd = cpp_array(MAXN);

var inp = cpp_array(MAXN);

class ConjDisj
{
  var n: dynamic;
  func ConjDisj(N: dynamic)
  {
      {
        var i = 0;
        var n = N;
        while ((i < n))
        {
          pai[i] = i;
          ranki[i] = 0;
          i += 1;
        }
      }
    }
  func busca(x: dynamic)
  {
      if ((x != pai[x]))
      {
        pai[x] = busca(pai[x]);
      }
      return pai[x];
    }
  func uniao(a: dynamic, b: dynamic)
  {
      if (((!inp[a]) || (!inp[b])))
      {
        return;
      }
      var paiA = busca(a);
      var paiB = busca(b);
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

var cd = cpp_construct((MAXN - 1));

func solve(mask: dynamic, W: dynamic)
{
  if ((!pd[mask]))
  {
    {
      var i = (1 << 21);
      while ((i > 0))
      {
        if ((i & mask))
        {
          var nmask = (mask ^ i);
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

func main()
{
  var n: dynamic;
  var m: dynamic;
  scanf(" %d %d", (&n), (&m));
  memset(pd, 0, cpp_sizeof(pd));
  {
    var i = 0;
    while ((i < m))
    {
      var x: dynamic;
      scanf(" %d", (&x));
      inp[x] = true;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (1 << 22)))
    {
      if ((!inp[i]))
      {
        i += 1;
        continue;
      }
      var x = i;
      var w = (((((1 << 22)) - 1)) ^ x);
      cd.uniao(x, w);
      solve(w, x);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (1 << 22)))
    {
      if (inp[i])
      {
        tmp[cd.busca(i)] = 1;
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
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
