// Translated from solution.cpp.

var maxn: dynamic = 110000;

var n: dynamic = cpp_uninitialized();

class Tinit
{
  var f: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
}

var A: dynamic = cpp_array(maxn);

class Tcxx
{
  var A: dynamic = cpp_uninitialized();
  var B: dynamic = cpp_uninitialized();
  func Tcxx(A: dynamic = 0, B: dynamic = 0) -> dynamic
  {
      A = A;
      B = B;
    }
  func operator_less(b: dynamic) -> dynamic
  {
      return  ((A != b.A)) ? (A < b.A) : (B < b.B);
    }
  func operator_add(b: dynamic) -> dynamic
  {
      return Tcxx((A + b.A), (B + b.B));
    }
}

class Talt
{
  var tot: dynamic = cpp_uninitialized();
  var pos: dynamic = cpp_uninitialized();
  var buf: dynamic = cpp_array(maxn);
  func add(a: dynamic, b: dynamic) -> dynamic
  {
      buf[cpp_update(tot, "++")].set(pos[a], b);
      pos[a] = (buf + tot);
    }
}

var alt: dynamic = cpp_uninitialized();

func init() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d%d", (&A[i].f), (&A[i].u));
      A[i].u -= 1;
      alt.add(A[i].f, i);
      i += 1;
    }
  }
}

var ans: dynamic = cpp_uninitialized();

var B: dynamic = cpp_uninitialized();

var U: dynamic = cpp_array(maxn);

var F: dynamic = cpp_array(2, maxn);

var G: dynamic = cpp_array(maxn);

var E: dynamic = cpp_uninitialized();

func dfs(x: dynamic, rt: dynamic) -> dynamic
{
  var p: dynamic = cpp_uninitialized();
  var ret: dynamic = cpp_uninitialized();
  U[x] = true;
  F[x][0] = cpp_assign(F[x][1], "=", Tcxx());
  {
    p = alt.pos[x];
    while (p)
    {
      if ((p->s != rt))
      {
        dfs(p->s, rt);
        F[x][1] = (F[x][1] + max(F[p->s][0], F[p->s][1]));
        ret = ((F[x][0] + F[p->s][0]) + Tcxx(1, (A[x].u ^ A[p->s].u)));
        if ((F[x][1] < ret))
        {
          F[x][1] = ret;
          G[x] = p->s;
        }
        F[x][0] = (F[x][0] + max(F[p->s][0], F[p->s][1]));
      }
      p = p->l;
    }
  }
}

func constrc(x: dynamic, c: dynamic, rt: dynamic) -> dynamic
{
  var p: dynamic = cpp_uninitialized();
  {
    p = alt.pos[x];
    while (p)
    {
      if ((p->s != rt))
      {
        if (((!c) || (p->s != G[x])))
        {
          constrc(p->s,  ((F[p->s][0] < F[p->s][1])) ? 1 : 0, rt);
        } else
        {
          E.push_back(Tcxx(x, p->s));
          constrc(p->s, 0, rt);
        }
      }
      p = p->l;
    }
  }
}

func solve(x: dynamic) -> dynamic
{
  var Fc: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  {
    while ((!U[x]))
    {
      U[x] = true;
      x = A[x].f;
    }
  }
  {
    i = 0;
    while ((i < 2))
    {
      dfs(x, x);
      if ((Fc < F[x][1]))
      {
        Fc = F[x][1];
        E.clear();
        constrc(x, 1, x);
      }
      i += 1;
      x = A[x].f;
    }
  }
  ans = (ans + Fc);
  B.insert(B.end(), E.begin(), E.end());
}

func solve() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  {
    i = 1;
    while ((i <= n))
    {
      if ((!U[i]))
      {
        Ntree.solve(i);
      }
      i += 1;
    }
  }
  printf("%d %d\n", ans.A, ans.B);
  {
    p = B.begin();
    while ((p != B.end()))
    {
      printf("%d %d\n", p->A, p->B);
      p += 1;
    }
  }
}

func main() -> dynamic
{
  Ninit.init();
  Nsolve.solve();
  return 0;
}
