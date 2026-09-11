// Translated from solution.cpp.

var ll: dynamic = dynamic;

var ull: dynamic = dynamic;

func rep(i: dynamic, n: dynamic, N: dynamic) -> dynamic
{
  cpp_macro("for(ll i=n;i<=N;++i)");
}

func rap(i: dynamic, n: dynamic, N: dynamic) -> dynamic
{
  cpp_macro("for(ll i=n;i>=N;--i)");
}

var mp: dynamic = cpp_expression("#include");

var pb: dynamic = cpp_expression("#include");

var pob: dynamic = cpp_expression("#include");

var pf: dynamic = cpp_expression("#include <");

var pof: dynamic = cpp_expression("#include");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

var ff: dynamic = cpp_expression("#incl");

var fs: dynamic = cpp_expression("#incl");

var sf: dynamic = cpp_expression("#incl");

var ss: dynamic = cpp_expression("#incl");

var lc: dynamic = cpp_expression("#includ");

var rc: dynamic = cpp_expression("#include <b");

func db(x: dynamic) -> dynamic
{
  cpp_macro("cout << \">>>>>> \" << #x << \" -> \" << x << endl;");
}

func all(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/st");
}

var pii: dynamic = cpp_expression("#include <bit");

var pll: dynamic = cpp_expression("#include <b");

var piii: dynamic = cpp_expression("#include <bit");

var piiii: dynamic = cpp_expression("#include <bit");

var psi: dynamic = cpp_expression("#include <bits/s");

var endl: dynamic = cpp_expression("#inc");

var MAX: dynamic = (1e5 + 5);

var MAX2: dynamic = 11;

var MOD: dynamic = 1000000007;

var INF: dynamic = 2e18;

var dr: dynamic = [1, 0, -1, 0, 1, 1, -1, -1, 0];

var dc: dynamic = [0, 1, 0, -1, 1, -1, 1, -1, 0];

var pi: dynamic = acos(-1);

var EPS: dynamic = 1e-9;

var block: dynamic = 450;

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var inv: dynamic = cpp_uninitialized();

var x: dynamic = cpp_array(MAX);

var y: dynamic = cpp_array(MAX);

var bit: dynamic = cpp_array(MAX);

var res: dynamic = cpp_uninitialized();

var id: dynamic = cpp_uninitialized();

func upd(i: dynamic, z: dynamic) -> dynamic
{
  {
    while ((i <= n))
    {
      bit[i] += z;
      i += ((i & (-i)));
    }
  }
}

var ret: dynamic = cpp_uninitialized();

func que(i: dynamic) -> dynamic
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

var pq: dynamic = cpp_uninitialized();

var tmp: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

func main() -> dynamic
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
    for (var i: dynamic in ans)
    {
      write(i, "\n");
    }
  }
  return 0;
}
