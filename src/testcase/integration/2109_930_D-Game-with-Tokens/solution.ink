// Translated from solution.cpp.

func operator_shift_right(i: dynamic, p: dynamic)
{
  ((i >> p.first) >> p.second);
  return i;
}

func operator_shift_right(i: dynamic, t: dynamic)
{
  for (var v in t)
  {
    (i >> v);
  }
  return i;
}

func operator_shift_left(o: dynamic, p: dynamic)
{
  (((o << p.first) << cpp_char(" ")) << p.second);
  return o;
}

func operator_shift_left(o: dynamic, t: dynamic)
{
  if (t.empty())
  {
    (o << cpp_char("\n"));
  }
  {
    var i = 0;
    while ((i < t.size()))
    {
      ((o << t[i]) << " \n"[(i == (t.size() - 1))]);
      i += 1;
    }
  }
  return o;
}

func in_cpp(a: dynamic, b: dynamic, c: dynamic)
{
  return ((a <= b) && (b < c));
}

func logceil(first: dynamic)
{
  return ((8 * cpp_sizeof(dynamic)) - builtin_clz(first));
}

class hash_pair_T_U
{
  var t: dynamic;
  var u: dynamic;
  func operator_call(p: dynamic)
  {
      return (t(p.first) ^ ((u(p.second) << 7)));
    }
}

func bsh(l: dynamic, h: dynamic, f: dynamic)
{
  var r = -1;
  var m: dynamic;
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

func bshd(l: dynamic, h: dynamic, f: dynamic, p: dynamic = 1e-9)
{
  var r = (3 + cpp_cast(log2((((h - l)) / p))));
  while (cpp_update(r, "--"))
  {
    var m = (((l + h)) / 2);
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

func bsl(l: dynamic, h: dynamic, f: dynamic)
{
  var r = -1;
  var m: dynamic;
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

func bsld(l: dynamic, h: dynamic, f: dynamic, p: dynamic = 1e-9)
{
  var r = (3 + cpp_cast(log2((((h - l)) / p))));
  while (cpp_update(r, "--"))
  {
    var m = (((l + h)) / 2);
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

func gcd(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    swap(a, b);
  }
  return if (b) gcd(b, (a % b)) else a;
}

class vector2
{
  func vector2()
  {
    }
  func vector2(a: dynamic, b: dynamic, t: dynamic = T())
  {
      this->vector_vector_T = cpp_construct(a, vector(b, t));
    }
}

class vector3
{
  func vector3()
  {
    }
  func vector3(a: dynamic, b: dynamic, c: dynamic, t: dynamic = T())
  {
      this->vector_vector2_T = cpp_construct(a, vector2(b, c, t));
    }
}

class vector4
{
  func vector4()
  {
    }
  func vector4(a: dynamic, b: dynamic, c: dynamic, d: dynamic, t: dynamic = T())
  {
      this->vector_vector3_T = cpp_construct(a, vector3(b, c, d, t));
    }
}

class vector5
{
  func vector5()
  {
    }
  func vector5(a: dynamic, b: dynamic, c: dynamic, d: dynamic, e: dynamic, t: dynamic = T())
  {
      this->vector_vector4_T = cpp_construct(a, vector4(b, c, d, e, t));
    }
}

class bounded_priority_queue
{
  func bounded_priority_queue(X: dynamic)
  {
      this->A = cpp_construct(X);
      this->B = cpp_construct(0);
      this->s = cpp_construct(0);
    }
  func push(L: dynamic, V: dynamic)
  {
      B = max(B, L);
      A[L].push(V);
      s += 1;
    }
  func top()
  {
      return A[B].front();
    }
  func pop()
  {
      s -= 1;
      A[B].pop();
      while (((B > 0) && A[B].empty()))
      {
        B -= 1;
      }
    }
  func empty()
  {
      return A[B].empty();
    }
  func clear()
  {
      s = cpp_assign(B, "=", 0);
      for (var a in A)
      {
        a = queue();
      }
    }
  func size()
  {
      return s;
    }
  var A: dynamic;
  var B: dynamic;
  var s: dynamic;
}

class TaskD
{
  func solve(cin: dynamic, cout: dynamic)
  {
      var N: dynamic;
      read(N);
      var Q = cpp_construct(2);
      {
        var i = 0;
        while ((i < N))
        {
          var first: dynamic;
          var second: dynamic;
          read(first, second);
          var d = ((((first + second)) & 1));
          Q[d].push_back([((((first + second) - d)) / 2), ((((first - second) - d)) / 2)]);
          i += 1;
        }
      }
      var ans = 0;
      for (var q in Q)
      {
        sort(q.begin(), q.end());
        if (q.empty())
        {
          continue;
        }
        var l = q[0].first;
        for (var qq in q)
        {
          qq.first -= l;
        }
        var h = q.back().first;
        var lo = cpp_construct((h + 1), 1000000);
        var hi = cpp_construct((h + 1), -1000000);
        for (var qq in q)
        {
          lo[qq.first] = min(lo[qq.first], qq.second);
          hi[qq.first] = max(hi[qq.first], qq.second);
        }
        var LL = lo;
        var LH = hi;
        var RL = lo;
        var RH = hi;
        {
          var i = 0;
          while ((i < h))
          {
            LL[(i + 1)] = min(LL[(i + 1)], LL[i]);
            LH[(i + 1)] = max(LH[(i + 1)], LH[i]);
            RL[((h - i) - 1)] = min(RL[((h - i) - 1)], RL[(h - i)]);
            RH[((h - i) - 1)] = max(RH[((h - i) - 1)], RH[(h - i)]);
            i += 1;
          }
        }
        {
          var i = 0;
          while ((i < h))
          {
            var lo = max(LL[i], RL[(i + 1)]);
            var hi = min(LH[i], RH[(i + 1)]);
            if ((lo < hi))
            {
              ans += (hi - lo);
            }
            i += 1;
          }
        }
      }
      write(ans, "\n");
    }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var solver: dynamic;
  var in_cpp: dynamic;
  var out: dynamic;
  solver.solve(in_cpp, out);
  return 0;
}
