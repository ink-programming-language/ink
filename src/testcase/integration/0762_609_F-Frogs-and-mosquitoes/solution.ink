// Translated from solution.cpp.

var inf: dynamic = 987654321;

var INF: dynamic = 123456789987654321;

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var Xn: dynamic = cpp_uninitialized();

class Frog
{
  var x: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
}

var frog: dynamic = cpp_uninitialized();

class Mosq
{
  var p: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
}

var mosq: dynamic = cpp_uninitialized();

class BIT
{
  var tree: dynamic = cpp_uninitialized();
  func init() -> dynamic
  {
      tree = vector((4 * Xn), pair(-1, -1));
    }
  func udt(idx: dynamic, val: dynamic, l: dynamic, r: dynamic, n: dynamic) -> dynamic
  {
      if (((idx < l) || (r < idx)))
      {
        return;
      }
      if ((l == r))
      {
        tree[n] = val;
        return;
      }
      var m: dynamic = (((l + r)) >> 1);
      udt(idx, val, l, m, (2 * n));
      udt(idx, val, (m + 1), r, ((2 * n) + 1));
      tree[n] = max(tree[(2 * n)], tree[((2 * n) + 1)]);
    }
  func add(idx: dynamic, val: dynamic, l: dynamic, r: dynamic, n: dynamic) -> dynamic
  {
      if (((idx < l) || (r < idx)))
      {
        return;
      }
      if ((l == r))
      {
        tree[n].first += val.first;
        tree[n].second += val.second;
        return;
      }
      var m: dynamic = (((l + r)) >> 1);
      add(idx, val, l, m, (2 * n));
      add(idx, val, (m + 1), r, ((2 * n) + 1));
      tree[n] = max(tree[(2 * n)], tree[((2 * n) + 1)]);
    }
  func left_most(k: dynamic, a: dynamic, b: dynamic, l: dynamic, r: dynamic, n: dynamic) -> dynamic
  {
      if (((b < l) || (r < a)))
      {
        return pair(-1, -1);
      }
      if (((a <= l) && (r <= b)))
      {
        if ((tree[n].first < k))
        {
          return pair(-1, -1);
        }
        if ((l == r))
        {
          if ((tree[n].first >= k))
          {
            return tree[n];
          } else
          {
            return pair(-1, -1);
          }
        }
        var m: dynamic = (((l + r)) >> 1);
        if ((tree[(2 * n)].first >= k))
        {
          return left_most(k, a, b, l, m, (2 * n));
        } else
        {
          return left_most(k, a, b, (m + 1), r, ((2 * n) + 1));
        }
      }
      var m: dynamic = (((l + r)) >> 1);
      var t: dynamic = left_most(k, a, b, l, m, (2 * n));
      if ((t != pair(-1, -1)))
      {
        return t;
      } else
      {
        return left_most(k, a, b, (m + 1), r, ((2 * n) + 1));
      }
    }
}

var bit: dynamic = cpp_uninitialized();

var bit2: dynamic = cpp_uninitialized();

var X: dynamic = cpp_uninitialized();

var dx: dynamic = cpp_uninitialized();

func compress() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      X.push_back(frog[i].x);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      X.push_back(mosq[i].p);
      i += 1;
    }
  }
  sort((X).begin(), (X).end());
  X.resize((unique((X).begin(), (X).end()) - X.begin()));
  Xn = X.size();
  {
    var i: dynamic = 0;
    while ((i < Xn))
    {
      dx[X[i]] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      frog[i].x = dx[frog[i].x];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      mosq[i].p = dx[mosq[i].p];
      i += 1;
    }
  }
}

var ans: dynamic = cpp_uninitialized();

var restore: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d %d", (&N), (&M));
  frog.resize(N);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      var x: dynamic = cpp_uninitialized();
      var t: dynamic = cpp_uninitialized();
      scanf("%d %d", (&x), (&t));
      frog[i] = [x, t, i];
      i += 1;
    }
  }
  mosq.resize(M);
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      var p: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      scanf("%d %d", (&p), (&b));
      mosq[i] = [p, b];
      i += 1;
    }
  }
  compress();
  bit.init();
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      bit.udt(frog[i].x, pair((X[frog[i].x] + cpp_cast(frog[i].t)), cpp_cast(frog[i].id)), 0, (Xn - 1), 1);
      i += 1;
    }
  }
  ans.resize(N);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      ans[i] = pair(0, cpp_cast(frog[i].t));
      i += 1;
    }
  }
  bit2.init();
  restore = vector(Xn, pair(0, 0));
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      var p: dynamic = mosq[i].p;
      var b: dynamic = mosq[i].b;
      var t: dynamic = bit.left_most(X[p], 0, p, 0, (Xn - 1), 1);
      if ((t == pair(-1, -1)))
      {
        bit2.udt(p, pair(1, p), 0, (Xn - 1), 1);
        restore[p].first += 1;
        restore[p].second += cpp_cast(b);
        i += 1;
        continue;
      }
      ans[t.second].first += 1;
      ans[t.second].second += cpp_cast(b);
      bit.add(frog[t.second].x, pair(cpp_cast(b), 0), 0, (Xn - 1), 1);
      while (1)
      {
        var t2: dynamic = bit2.left_most(1, frog[t.second].x, (Xn - 1), 0, (Xn - 1), 1);
        if ((t2 == pair(-1, -1)))
        {
          break;
        }
        if ((X[t2.second] > (cpp_cast(X[frog[t.second].x]) + cpp_cast(ans[t.second].second))))
        {
          break;
        }
        bit2.udt(t2.second, pair(-1, -1), 0, (Xn - 1), 1);
        ans[t.second].first += restore[t2.second].first;
        ans[t.second].second += restore[t2.second].second;
        bit.add(frog[t.second].x, pair(restore[t2.second].second, 0), 0, (Xn - 1), 1);
        restore[t2.second] = pair(0, 0);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      printf("%I64d %I64d\n", ans[i].first, ans[i].second);
      i += 1;
    }
  }
}
