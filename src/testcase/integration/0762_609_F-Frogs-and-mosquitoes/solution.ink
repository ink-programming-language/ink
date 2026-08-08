// Translated from solution.cpp.

var inf = 987654321;

var INF = 123456789987654321;

var N: dynamic;

var M: dynamic;

var Xn: dynamic;

class Frog
{
  var x: dynamic;
  var t: dynamic;
  var id: dynamic;
}

var frog: dynamic;

class Mosq
{
  var p: dynamic;
  var b: dynamic;
}

var mosq: dynamic;

class BIT
{
  var tree: dynamic;
  func init()
  {
      tree = vector((4 * Xn), pair(-1, -1));
    }
  func udt(idx: dynamic, val: dynamic, l: dynamic, r: dynamic, n: dynamic)
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
      var m = (((l + r)) >> 1);
      udt(idx, val, l, m, (2 * n));
      udt(idx, val, (m + 1), r, ((2 * n) + 1));
      tree[n] = max(tree[(2 * n)], tree[((2 * n) + 1)]);
    }
  func add(idx: dynamic, val: dynamic, l: dynamic, r: dynamic, n: dynamic)
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
      var m = (((l + r)) >> 1);
      add(idx, val, l, m, (2 * n));
      add(idx, val, (m + 1), r, ((2 * n) + 1));
      tree[n] = max(tree[(2 * n)], tree[((2 * n) + 1)]);
    }
  func left_most(k: dynamic, a: dynamic, b: dynamic, l: dynamic, r: dynamic, n: dynamic)
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
        var m = (((l + r)) >> 1);
        if ((tree[(2 * n)].first >= k))
        {
          return left_most(k, a, b, l, m, (2 * n));
        } else
        {
          return left_most(k, a, b, (m + 1), r, ((2 * n) + 1));
        }
      }
      var m = (((l + r)) >> 1);
      var t = left_most(k, a, b, l, m, (2 * n));
      if ((t != pair(-1, -1)))
      {
        return t;
      } else
      {
        return left_most(k, a, b, (m + 1), r, ((2 * n) + 1));
      }
    }
}

var bit: dynamic;

var bit2: dynamic;

var X: dynamic;

var dx: dynamic;

func compress()
{
  {
    var i = 0;
    while ((i < N))
    {
      X.push_back(frog[i].x);
      i += 1;
    }
  }
  {
    var i = 0;
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
    var i = 0;
    while ((i < Xn))
    {
      dx[X[i]] = i;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      frog[i].x = dx[frog[i].x];
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < M))
    {
      mosq[i].p = dx[mosq[i].p];
      i += 1;
    }
  }
}

var ans: dynamic;

var restore: dynamic;

func main()
{
  scanf("%d %d", (&N), (&M));
  frog.resize(N);
  {
    var i = 0;
    while ((i < N))
    {
      var x: dynamic;
      var t: dynamic;
      scanf("%d %d", (&x), (&t));
      frog[i] = [x, t, i];
      i += 1;
    }
  }
  mosq.resize(M);
  {
    var i = 0;
    while ((i < M))
    {
      var p: dynamic;
      var b: dynamic;
      scanf("%d %d", (&p), (&b));
      mosq[i] = [p, b];
      i += 1;
    }
  }
  compress();
  bit.init();
  {
    var i = 0;
    while ((i < N))
    {
      bit.udt(frog[i].x, pair((X[frog[i].x] + cpp_cast(frog[i].t)), cpp_cast(frog[i].id)), 0, (Xn - 1), 1);
      i += 1;
    }
  }
  ans.resize(N);
  {
    var i = 0;
    while ((i < N))
    {
      ans[i] = pair(0, cpp_cast(frog[i].t));
      i += 1;
    }
  }
  bit2.init();
  restore = vector(Xn, pair(0, 0));
  {
    var i = 0;
    while ((i < M))
    {
      var p = mosq[i].p;
      var b = mosq[i].b;
      var t = bit.left_most(X[p], 0, p, 0, (Xn - 1), 1);
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
        var t2 = bit2.left_most(1, frog[t.second].x, (Xn - 1), 0, (Xn - 1), 1);
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
    var i = 0;
    while ((i < N))
    {
      printf("%I64d %I64d\n", ans[i].first, ans[i].second);
      i += 1;
    }
  }
}
