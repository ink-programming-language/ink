// Translated from solution.cpp.

var ll = dynamic;

var ull = dynamic;

func rep(i: dynamic, n: dynamic, N: dynamic)
{
  cpp_macro("for(ll i=n;i<=N;++i)");
}

func rap(i: dynamic, n: dynamic, N: dynamic)
{
  cpp_macro("for(ll i=n;i>=N;--i)");
}

var mp = cpp_expression("#include");

var pb = cpp_expression("#include");

var pob = cpp_expression("#include");

var pf = cpp_expression("#include <");

var pof = cpp_expression("#include");

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

var ff = cpp_expression("#incl");

var fs = cpp_expression("#incl");

var sf = cpp_expression("#incl");

var ss = cpp_expression("#incl");

var lc = cpp_expression("#includ");

var rc = cpp_expression("#include <b");

func db(x: dynamic)
{
  cpp_macro("cout << \">>>>>> \" << #x << \" -> \" << x << endl;");
}

func all(x: dynamic)
{
  return cpp_expression("#include <bits/st");
}

var pii = cpp_expression("#include <bit");

var pll = cpp_expression("#include <b");

var piii = cpp_expression("#include <bit");

var piiii = cpp_expression("#include <bit");

var psi = cpp_expression("#include <bits/s");

var endl = cpp_expression("#inc");

var MAX = (1e5 + 5);

var MAX2 = 11;

var MOD = 1000000007;

var INF = 2e18;

var dr = [1, 0, -1, 0, 1, 1, -1, -1, 0];

var dc = [0, 1, 0, -1, 1, -1, 1, -1, 0];

var pi = acos(-1);

var EPS = 1e-9;

var block = 450;

var n: dynamic;

var k: dynamic;

var inv: dynamic;

var x = cpp_array(MAX);

var y = cpp_array(MAX);

var bit = cpp_array(MAX);

var res: dynamic;

var id: dynamic;

func upd(i: dynamic, z: dynamic)
{
  {
    while ((i <= n))
    {
      bit[i] += z;
      i += ((i & (-i)));
    }
  }
}

var ret: dynamic;

func que(i: dynamic)
{
  i -= 1;
  ret = 0;
  {
    while ((i > 0))
    {
      ret += bit[i];
      i -= ((i & (-i)));
    }
  }
  return ret;
}

var pq: dynamic;

var tmp: dynamic;

var ans: dynamic;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n, k);
  rep(i, 1, n);
  read(y[i]);
  x[y[i]] = i;
  rap(i, n, 1) += que(x[i]);
  upd(x[i], 1);
  if ((inv <= k))
  {
    ((rep(i, 1, n) << y[i]) << endl);
  } else
  {
    k = (inv - k);
    rep(i, 1, n).push(i);
    rep(i, 1, n);
    {
      while (((!tmp.empty()) && (que(x[tmp.top().se]) <= k)))
      {
        pq.push(tmp.top().se);
        tmp.pop();
      }
      while ((!pq.empty()))
      {
        id = pq.top();
        pq.pop();
        res = que(x[id]);
        if ((res > k))
        {
          tmp.push([x[id], id]);
        } else
        {
          ans.pb(id);
          k -= res;
          upd(x[id], -1);
          break;
        }
      }
    }
    for (var i in ans)
    {
      write(i, "\n");
    }
  }
  return 0;
}
