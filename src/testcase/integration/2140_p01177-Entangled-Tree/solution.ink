// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = 0; i < (int)(n); i++)");
}

var mp: dynamic = cpp_expression("#include");

var NUM: dynamic = cpp_expression("#include");

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var Q: dynamic = cpp_uninitialized();

var Y: dynamic = cpp_array(NUM);

var ls: dynamic = cpp_array(NUM);

var ord: dynamic = cpp_array(NUM);

var hi: dynamic = cpp_array(NUM);

var p: dynamic = cpp_array(NUM);

var c: dynamic = cpp_array(NUM);

var f: dynamic = cpp_array(NUM);

var b: dynamic = cpp_array(NUM);

var ans: dynamic = cpp_array(NUM);

var a: dynamic = cpp_array(NUM);

func fix(k: dynamic) -> dynamic
{
  sort(a[k].begin(), a[k].end());
  f[k] = a[k][0].first;
  c[k] = 0;
  rep(i, a[k].size())[k] +=  ((a[k][i].second == -1)) ? 1 : c[a[k][i].second];
}

func build() -> dynamic
{
  Y[M] = -1;
  rep(i, N)[i] = -1;
  rep(i, (M + 1))[i] = mp(Y[i], i);
  sort(ord, ((ord + M) + 1));
  {
    var ik: dynamic = M;
    while ((ik > 0))
    {
      var k: dynamic = ord[ik].second;
      a[k].clear();
      p[k] = -1;
      rep(i, ls[k].size());
      {
        var ix: dynamic = (ls[k][i] - 1);
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
    var k: dynamic = ord[ik].second;
    var z: dynamic = b[k];
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

func main() -> dynamic
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        scanf("%d", (Y + i));
        var L: dynamic = cpp_uninitialized();
        scanf("%d", (&L));
        ls[i].resize(L);
        rep(j, L);
        scanf("%d", (&ls[i][j]));
      }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var q: dynamic = cpp_uninitialized();
        scanf("%d", (&q));
        printf("%d\n", (ans[(q - 1)] + 1));
      }
