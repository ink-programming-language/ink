// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for (int i = 0; i < (int)(n); i++)");
}

var mp = cpp_expression("#include");

var NUM = cpp_expression("#include");

var N: dynamic;

var M: dynamic;

var Q: dynamic;

var Y = cpp_array(NUM);

var ls = cpp_array(NUM);

var ord = cpp_array(NUM);

var hi = cpp_array(NUM);

var p = cpp_array(NUM);

var c = cpp_array(NUM);

var f = cpp_array(NUM);

var b = cpp_array(NUM);

var ans = cpp_array(NUM);

var a = cpp_array(NUM);

func fix(k: dynamic)
{
  sort(a[k].begin(), a[k].end());
  f[k] = a[k][0].first;
  c[k] = 0;
  rep(i, a[k].size())[k] += if ((a[k][i].second == -1)) 1 else c[a[k][i].second];
}

func build()
{
  Y[M] = -1;
  rep(i, N)[i] = -1;
  rep(i, (M + 1))[i] = mp(Y[i], i);
  sort(ord, ((ord + M) + 1));
  {
    var ik = M;
    while ((ik > 0))
    {
      var k = ord[ik].second;
      a[k].clear();
      p[k] = -1;
      rep(i, ls[k].size());
      {
        var ix = (ls[k][i] - 1);
        if ((hi[ix] == -1))
        {
          a[k].push_back(mp(ix, -1));
        } else
        {
          a[k].push_back(mp(f[hi[ix]], hi[ix]));
          assert((p[hi[ix]] == -1));
          p[hi[ix]] = i;
        }
        hi[ix] = k;
      }
      fix(k);
      ik -= 1;
    }
  }
  a[M].clear();
  rep(i, N);
  if ((hi[i] == -1))
  {
    a[M].push_back(mp(i, -1));
  }
  rep(i, M);
  if ((p[i] == -1))
  {
    a[M].push_back(mp(f[i], i));
  }
  fix(M);
  b[M] = 0;
  rep(ik, (M + 1));
  {
    var k = ord[ik].second;
    var z = b[k];
    rep(i, a[k].size());
    {
      if ((a[k][i].second == -1))
      {
        ans[z] = a[k][i].first;
        z += 1;
      } else
      {
        b[a[k][i].second] = z;
        z += c[a[k][i].second];
      }
    }
  }
}

func main()
{
  {
    while (true)
    {
      scanf("%d%d%d", (&N), (&M), (&Q));
      if ((N == 0))
      {
        return 0;
      }
      build();
      printf("\n");
    }
  }
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
        scanf("%d", (Y + i));
        var L: dynamic;
        scanf("%d", (&L));
        ls[i].resize(L);
        rep(j, L);
        scanf("%d", (&ls[i][j]));
      }

func rep(argument_0: dynamic, argument_1: dynamic)
{
        var q: dynamic;
        scanf("%d", (&q));
        printf("%d\n", (ans[(q - 1)] + 1));
      }
