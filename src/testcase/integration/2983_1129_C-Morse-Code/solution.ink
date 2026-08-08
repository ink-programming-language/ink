// Translated from solution.cpp.

var MOD = (1e9 + 7);

var INF = 0x3f3f3f3f;

var LINF = 0x3f3f3f3f3f3f3f3f;

var N: dynamic;

var n: dynamic;

var SA = cpp_array(3003);

var lcp = cpp_array(3003);

var arr = cpp_array(3003);

var rsa = cpp_array(3003);

var D = cpp_array(3003, 3003);

var T = cpp_array(3003);

var S = cpp_array(3003);

func SuffixArray()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var m = 2;
  var cnt = cpp_construct((max(N, m) + 1), 0);
  var first = cpp_construct((N + 1), 0);
  var second = cpp_construct((N + 1), 0);
  {
    i = 1;
    while ((i <= N))
    {
      cnt[cpp_assign(first[i], "=", ((S[i] - cpp_char("a")) + 1))] += 1;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= m))
    {
      cnt[i] += cnt[(i - 1)];
      i += 1;
    }
  }
  {
    i = N;
    while (i)
    {
      SA[cpp_update(cnt[first[i]], "--")] = i;
      i -= 1;
    }
  }
  {
    var len = 1;
    var p = 1;
    while ((p < N))
    {
      {
        p = 0;
        i = (N - len);
        while ((cpp_update(i, "++") <= N))
        {
          second[cpp_update(p, "++")] = i;
        }
      }
      {
        i = 1;
        while ((i <= N))
        {
          if ((SA[i] > len))
          {
            second[cpp_update(p, "++")] = (SA[i] - len);
          }
          i += 1;
        }
      }
      {
        i = 0;
        while ((i <= m))
        {
          cnt[i] = 0;
          i += 1;
        }
      }
      {
        i = 1;
        while ((i <= N))
        {
          cnt[first[second[i]]] += 1;
          i += 1;
        }
      }
      {
        i = 1;
        while ((i <= m))
        {
          cnt[i] += cnt[(i - 1)];
          i += 1;
        }
      }
      {
        i = N;
        while (i)
        {
          SA[cpp_update(cnt[first[second[i]]], "--")] = second[i];
          i -= 1;
        }
      }
      swap(first, second);
      p = 1;
      first[SA[1]] = 1;
      {
        i = 1;
        while ((i < N))
        {
          first[SA[(i + 1)]] = if ((((((SA[i] + len) <= N) && ((SA[(i + 1)] + len) <= N)) && (second[SA[i]] == second[SA[(i + 1)]])) && (second[(SA[i] + len)] == second[(SA[(i + 1)] + len)]))) p else cpp_update(p, "++");
          i += 1;
        }
      }
      len <<= 1;
      m = p;
    }
  }
}

func LCP()
{
  var i: dynamic;
  var j: dynamic;
  var k = 0;
  var rank = cpp_construct((N + 1), 0);
  {
    i = 1;
    while ((i <= N))
    {
      rank[SA[i]] = i;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= N))
    {
      {
        if (k) cpp_update(k, "--") else 0;
        j = SA[(rank[i] - 1)];
        while ((S[(i + k)] == S[(j + k)]))
        {
          k += 1;
        }
      }
      lcp[rank[cpp_update(i, "++")]] = k;
    }
  }
}

var bstr = ["0011", "0101", "1110", "1111"];

func bad(s: dynamic, e: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  {
    j = 0;
    while ((j < 4))
    {
      {
        i = s;
        while ((i <= e))
        {
          if (((bstr[j][(i - s)] - cpp_char("0")) != arr[i]))
          {
            break;
          }
          i += 1;
        }
      }
      if ((i == (e + 1)))
      {
        return 1;
      }
      j += 1;
    }
  }
  return 0;
}

var rns = cpp_array(3003);

func fin(idx: dynamic)
{
  var i: dynamic;
  var p: dynamic;
  {
    i = 1;
    while ((i <= n))
    {
      if ((SA[i] == idx))
      {
        break;
      }
      i += 1;
    }
  }
  p = i;
  var t = lcp[p];
  {
    i = (p - 1);
    while (i)
    {
      rns[SA[i]] = t;
      t = min(t, lcp[i]);
      i -= 1;
    }
  }
  t = lcp[(p + 1)];
  {
    i = (p + 1);
    while ((i <= n))
    {
      rns[SA[i]] = t;
      t = min(t, lcp[(i + 1)]);
      i += 1;
    }
  }
  var maxi = 0;
  {
    i = 1;
    while ((i < idx))
    {
      maxi = max(maxi, rns[i]);
      i += 1;
    }
  }
  return (idx - maxi);
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  scanf("%d", (&n));
  N = n;
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&arr[i]));
      i += 1;
    }
  }
  {
    i = n;
    while ((i > 0))
    {
      S[((n + 1) - i)] = (cpp_char("a") + arr[i]);
      i -= 1;
    }
  }
  SuffixArray();
  LCP();
  {
    i = 1;
    while ((i <= n))
    {
      SA[i] = ((n + 1) - SA[i]);
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= i))
        {
          {
            k = 0;
            while (((k <= 3) && (k < i)))
            {
              if (((k == 3) && bad((i - k), i)))
              {
                k += 1;
                continue;
              }
              D[i][j] = (((D[i][j] + D[((i - k) - 1)][min(j, ((i - k) - 1))])) % MOD);
              if (((i - k) <= j))
              {
                D[i][j] += 1;
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      var t = fin(i);
      T[i] = (((T[(i - 1)] + D[i][t])) % MOD);
      printf("%lld\n", T[i]);
      i += 1;
    }
  }
  return 0;
}
