// Translated from solution.cpp.

func operator_shift_right(i: dynamic, p: dynamic) -> dynamic
{
  ((i >> p.first) >> p.second);
  return i;
}

func operator_shift_right(i: dynamic, t: dynamic) -> dynamic
{
  for (var v: dynamic in t)
  {
    (i >> v);
  }
  return i;
}

func operator_shift_left(o: dynamic, p: dynamic) -> dynamic
{
  (((o << p.first) << cpp_char(" ")) << p.second);
  return o;
}

func operator_shift_left(o: dynamic, t: dynamic) -> dynamic
{
  if (t.empty())
  {
    (o << cpp_char("\n"));
  }
  {
    var i: dynamic = 0;
    while ((i < t.size()))
    {
      ((o << t[i]) << " \n"[(i == (t.size() - 1))]);
      i += 1;
    }
  }
  return o;
}

func logceil(first: dynamic) -> dynamic
{
  return  (first) ? ((8 * cpp_sizeof(dynamic)) - builtin_clzll(first)) : 0;
}

class hash_pair_T_U
{
  var t: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
  func operator_call(p: dynamic) -> dynamic
  {
      return (t(p.first) ^ ((u(p.second) << 7)));
    }
}

func bsh(l: dynamic, h: dynamic, f: dynamic) -> dynamic
{
  var r: dynamic = -1;
  var m: dynamic = cpp_uninitialized();
  while ((l <= h))
  {
    m = (((l + h)) / 2);
    if (f(m))
    {
      l = (m + 1);
      r = m;
    } else
    {
      h = (m - 1);
    }
  }
  return r;
}

func bshd(l: dynamic, h: dynamic, f: dynamic, p: dynamic = 1e-9) -> dynamic
{
  var r: dynamic = (3 + cpp_cast(log2((((h - l)) / p))));
  while (cpp_update(r, "--"))
  {
    var m: dynamic = (((l + h)) / 2);
    if (f(m))
    {
      l = m;
    } else
    {
      h = m;
    }
  }
  return (((l + h)) / 2);
}

func bsl(l: dynamic, h: dynamic, f: dynamic) -> dynamic
{
  var r: dynamic = -1;
  var m: dynamic = cpp_uninitialized();
  while ((l <= h))
  {
    m = (((l + h)) / 2);
    if (f(m))
    {
      h = (m - 1);
      r = m;
    } else
    {
      l = (m + 1);
    }
  }
  return r;
}

func bsld(l: dynamic, h: dynamic, f: dynamic) -> dynamic
{
  var r: dynamic = 200;
  while (cpp_update(r, "--"))
  {
    var m: dynamic = (((l + h)) / 2);
    if (f(m))
    {
      h = m;
    } else
    {
      l = m;
    }
  }
  return (((l + h)) / 2);
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    swap(a, b);
  }
  return  (b) ? gcd(b, (a % b)) : a;
}

class vector2
{
  func vector2() -> dynamic
  {
    }
  func vector2(a: dynamic, b: dynamic, t: dynamic = T()) -> dynamic
  {
      self->vector_vector_T = cpp_construct(a, vector(b, t));
    }
}

class vector3
{
  func vector3() -> dynamic
  {
    }
  func vector3(a: dynamic, b: dynamic, c: dynamic, t: dynamic = T()) -> dynamic
  {
      self->vector_vector2_T = cpp_construct(a, vector2(b, c, t));
    }
}

class vector4
{
  func vector4() -> dynamic
  {
    }
  func vector4(a: dynamic, b: dynamic, c: dynamic, d: dynamic, t: dynamic = T()) -> dynamic
  {
      self->vector_vector3_T = cpp_construct(a, vector3(b, c, d, t));
    }
}

class vector5
{
  func vector5() -> dynamic
  {
    }
  func vector5(a: dynamic, b: dynamic, c: dynamic, d: dynamic, e: dynamic, t: dynamic = T()) -> dynamic
  {
      self->vector_vector4_T = cpp_construct(a, vector4(b, c, d, e, t));
    }
}

class GOmkarAndPies
{
  func solve(cin: dynamic, cout: dynamic) -> dynamic
  {
      var N: dynamic = cpp_uninitialized();
      var M: dynamic = cpp_uninitialized();
      var K: dynamic = cpp_uninitialized();
      read(N, M, K);
      var S: dynamic = cpp_uninitialized();
      var T: dynamic = cpp_uninitialized();
      read(S, T);
      read(P);
      for (var p: dynamic in P)
      {
        p.first -= 1;
        p.second -= 1;
      }
      var TVal: dynamic = cpp_uninitialized();
      var SVal: dynamic = cpp_uninitialized();
      var Sx: dynamic = cpp_uninitialized();
      var Tx: dynamic = cpp_uninitialized();
      iota(X.begin(), X.end(), 0);
      {
        var i: dynamic = 0;
        while ((i < K))
        {
          Sx.push_back((S[i] - cpp_char("0")));
          Tx.push_back((T[i] - cpp_char("0")));
          i += 1;
        }
      }
      var apply: dynamic = __cpp_lambda_1;
      SVal.push_back(apply(Sx, X));
      TVal.push_back(apply(Tx, X));
      {
        var i: dynamic = 0;
        while ((i < N))
        {
          swap(X[P[i].first], X[P[i].second]);
          SVal.push_back(apply(Sx, X));
          TVal.push_back(apply(Tx, X));
          i += 1;
        }
      }
      var D: dynamic = cpp_construct((1 << K), [(K + 1), -1]);
      var Fix: dynamic = cpp_uninitialized();
      Fix.reserve((1 << K));
      var ans: dynamic = [(K + 1), [-1, -1]];
      {
        var i: dynamic = (N - M);
        while ((i >= 0))
        {
          var TX: dynamic = TVal[(i + M)];
          D[TX] = [0, (i + M)];
          Fix.push_back(TX);
          while ((!Fix.empty()))
          {
            var p: dynamic = Fix.back();
            Fix.pop_back();
            {
              var j: dynamic = 0;
              while ((j < K))
              {
                var s: dynamic = (p ^ (1 << j));
                if ((D[s].first > (D[p].first + 1)))
                {
                  D[s] = [(D[p].first + 1), D[p].second];
                  Fix.push_back(s);
                }
                j += 1;
              }
            }
          }
          var SX: dynamic = SVal[i];
          ans = min(ans, [D[SX].first, [(i + 1), D[SX].second]]);
          i -= 1;
        }
      }
      write((K - ans.first), cpp_char("\n"), ans.second, cpp_char("\n"));
    }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var solver: dynamic = cpp_uninitialized();
  var in_cpp: dynamic = cpp_uninitialized();
  var out: dynamic = cpp_uninitialized();
  solver.solve(in_cpp, out);
  return 0;
}

func __cpp_lambda_1(S: dynamic, V: dynamic) -> dynamic
{
  var p: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < K))
    {
      p |= (S[i] << V[i]);
      i += 1;
    }
  }
  return p;
}
