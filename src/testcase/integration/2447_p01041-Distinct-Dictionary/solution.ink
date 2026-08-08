// Translated from solution.cpp.

func repl(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(int)(a);i<(int)(b);(i)++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <b");
}

func dbg(x: dynamic)
{
  return cpp_expression("#include <bits/stdc++.");
}

var INF = cpp_expression("#include");

class RangeMinQuery
{
  var dat: dynamic = cpp_array((((1 << 19)) - 1));
  var size: dynamic;
  func init(n: dynamic)
  {
      size = 1;
      while ((size < n))
      {
        size *= 2;
      }
      {
        var i = 0;
        while ((i < ((2 * size) - 1)))
        {
          dat[i] = INF;
          i += 1;
        }
      }
    }
  func update(k: dynamic, a: dynamic)
  {
      k += (size - 1);
      dat[k] = a;
      while ((k > 0))
      {
        k = (((k - 1)) / 2);
        dat[k] = min(dat[((k * 2) + 1)], dat[((k * 2) + 2)]);
      }
    }
  func subquery(a: dynamic, b: dynamic, k: dynamic, l: dynamic, r: dynamic)
  {
      if (((r <= a) || (b <= l)))
      {
        return INF;
      }
      if (((a <= l) && (r <= b)))
      {
        return dat[k];
      } else
      {
        return min(subquery(a, b, ((k * 2) + 1), l, (((l + r)) / 2)), subquery(a, b, ((k * 2) + 2), (((l + r)) / 2), r));
      }
    }
  func query(a: dynamic, b: dynamic)
  {
      return subquery(a, b, 0, 0, size);
    }
}

class BIT
{
  var n: dynamic;
  var bit: dynamic;
  func BIT(size: dynamic)
  {
      this->n = cpp_construct(size);
      this->bit = cpp_construct((size + 1), 0);
    }
  func sum(i: dynamic)
  {
      var s = 0;
      while ((i > 0))
      {
        s += bit[i];
        i -= (i & (-i));
      }
      return s;
    }
  func add(i: dynamic, v: dynamic)
  {
      if ((i == 0))
      {
        return;
      }
      while ((i <= n))
      {
        bit[i] += v;
        i += (i & (-i));
      }
    }
  func lower_bound(w: dynamic)
  {
      if ((w <= 0))
      {
        return 0;
      }
      var x = 0;
      var r = 1;
      while ((r < n))
      {
        r <<= 1;
      }
      {
        var k = r;
        while ((k > 0))
        {
          if ((((x + k) <= n) && (bit[(x + k)] < w)))
          {
            w -= bit[(x + k)];
            x += k;
          }
          k >>= 1;
        }
      }
      return (x + 1);
    }
}

var N: dynamic;

var Q: dynamic;

var input = cpp_array(1000010);

var S: dynamic;

var ord: dynamic;

var ridx = cpp_array(100010);

var lcp = cpp_array(100010);

var rmq: dynamic;

func main()
{
  scanf("%d", (&N));
  S.resize(N);
  sort(ord.begin(), ord.end(), __cpp_lambda_1);
  rmq.init((N - 1));
  rep(i, (N - 1));
  {
    lcp[i] = 0;
    rep(j, min(S[ord[i]].size(), S[ord[(i + 1)]].size()));
    {
      if ((S[ord[i]][j] == S[ord[(i + 1)]][j]))
      {
        lcp[i] += 1;
      } else
      {
        break;
      }
    }
    rmq.update(i, lcp[i]);
  }
  scanf("%d", (&Q));
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    scanf("%s", input);
    S[i] = string_cpp(input);
    ord.push_back(i);
  }

func __cpp_lambda_1(a: dynamic, b: dynamic)
{
  return (S[a] < S[b]);
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    ridx[ord[i]] = i;
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var a: dynamic;
    var b: dynamic;
    scanf("%d%d", (&a), (&b));
    a -= 1;
    b -= 1;
    if ((a == 0))
    {
      bit.add((ridx[b] + 1), +1);
    } else if ((a == 1))
    {
      bit.add((ridx[b] + 1), -1);
    } else
    {
      var sum = bit.sum(ridx[b]);
      var idx = bit.lower_bound((sum + 1));
      if ((idx == (N + 1)))
      {
        write(-1, "\n");
        continue;
      }
      var len = rmq.query(ridx[b], (idx - 1));
      if ((len >= cpp_cast(S[b].size())))
      {
        write((ord[(idx - 1)] + 1), "\n");
      } else
      {
        write(-1, "\n");
      }
    }
  }
