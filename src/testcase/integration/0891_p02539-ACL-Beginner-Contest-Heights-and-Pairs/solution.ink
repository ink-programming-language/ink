// Translated from solution.cpp.

var MOD = cpp_expression("#include <bits/s");

class NTT
{
  var base: dynamic;
  var maxb: dynamic;
  var root: dynamic;
  var rv: dynamic;
  var roots: dynamic;
  var invr: dynamic;
  func NTT()
  {
      this->base = cpp_construct(1);
      this->rv = cpp_construct([0, 1]);
      this->roots = cpp_construct([0, 1]);
      this->invr = cpp_construct([0, 1]);
      assert(((mod >= 3) && (mod & 1)));
      var tmp = (mod - 1);
      maxb = 0;
      while ((!((tmp & 1))))
      {
        tmp >>= 1;
        maxb += 1;
      }
      root = 2;
      while ((mpow(root, (((mod - 1)) >> 1)) == 1))
      {
        root += 1;
      }
      assert((mpow(root, (mod - 1)) == 1));
      root = mpow(root, (((mod - 1)) >> maxb));
    }
  func mpow(x: dynamic, n: dynamic)
  {
      var res = 1;
      while (n)
      {
        if ((n & 1))
        {
          res = mul(res, x);
        }
        x = mul(x, x);
        n >>= 1;
      }
      return res;
    }
  func inv(x: dynamic)
  {
      return mpow(x, (mod - 2));
    }
  func add(x: dynamic, y: dynamic)
  {
      if (((cpp_assign(x, "+=", y)) >= mod))
      {
        x -= mod;
      }
      return x;
    }
  func mul(x: dynamic, y: dynamic)
  {
      return cpp_cast(((((1 * x) * y) % mod)));
    }
  func ensure_base(nb: dynamic)
  {
      if ((nb <= base))
      {
        return;
      }
      rv.resize((1 << nb));
      roots.resize((1 << nb));
      invr.resize((1 << nb));
      {
        var i = 0;
        while ((i < ((1 << nb))))
        {
          rv[i] = (((rv[(i >> 1)] >> 1)) + ((((i & 1)) << ((nb - 1)))));
          i += 1;
        }
      }
      assert((nb <= maxb));
      while ((base < nb))
      {
        var z = mpow(root, (1 << (((maxb - 1) - base))));
        var invz = inv(z);
        {
          var i = (1 << ((base - 1)));
          while ((i < ((1 << base))))
          {
            roots[(i << 1)] = roots[i];
            roots[(((i << 1)) + 1)] = mul(roots[i], z);
            invr[(i << 1)] = invr[i];
            invr[(((i << 1)) + 1)] = mul(invr[i], invz);
            i += 1;
          }
        }
        base += 1;
      }
    }
  func ntt(a: dynamic, n: dynamic, sg: dynamic = 0)
  {
      assert((((n & ((n - 1)))) == 0));
      {
        var i = 0;
        while ((i < n))
        {
          if ((i < rv[i]))
          {
            swap(a[i], a[rv[i]]);
          }
          i += 1;
        }
      }
      {
        var k = 1;
        while ((k < n))
        {
          {
            var i = 0;
            while ((i < n))
            {
              {
                var j = 0;
                while ((j < k))
                {
                  var z = mul(a[((i + j) + k)], (if (sg) roots[(j + k)] else invr[(j + k)]));
                  a[((i + j) + k)] = add(a[(i + j)], (mod - z));
                  a[(i + j)] = add(a[(i + j)], z);
                  j += 1;
                }
              }
              i += (2 * k);
            }
          }
          k <<= 1;
        }
      }
      var invn = inv(n);
      if (sg)
      {
        {
          var i = 0;
          while ((i < n))
          {
            a[i] = mul(a[i], invn);
            i += 1;
          }
        }
      }
    }
  func multiply(a: dynamic, b: dynamic)
  {
      var need = ((a.size() + b.size()) - 1);
      var nb = 1;
      while ((((1 << nb)) < need))
      {
        nb += 1;
      }
      ensure_base(nb);
      var sz = (1 << nb);
      var fa = cpp_construct(sz, 0);
      var fb = cpp_construct(sz, 0);
      {
        var i = 0;
        while ((i < sz))
        {
          if ((i < a.size()))
          {
            fa[i] = a[i];
          }
          if ((i < b.size()))
          {
            fb[i] = b[i];
          }
          i += 1;
        }
      }
      ntt(fa, sz);
      ntt(fb, sz);
      {
        var i = 0;
        while ((i < sz))
        {
          fa[i] = mul(fa[i], fb[i]);
          i += 1;
        }
      }
      ntt(fa, sz, 1);
      {
        var i = 0;
        while ((i < need))
        {
          res[i] = fa[i];
          i += 1;
        }
      }
      return res;
    }
}

var n: dynamic;

var memo: dynamic;

var pq: dynamic;

var ntt: dynamic;

func main()
{
  read(n);
  {
    var mp: dynamic;
    {
      var i = 0;
      while ((i < (2 * n)))
      {
        var x: dynamic;
        read(x);
        mp[x] += 1;
        i += 1;
      }
    }
    for (var __cpp_item_1 in mp)
    {
      var (cpp_name, p) = __cpp_item_1;
      var v = cpp_construct(1, 1);
      var now = 1;
      var cnt = 1;
      {
        var i = p;
        while ((i > 1))
        {
          (cpp_assign(now, "*=", cpp_update(i, "--"))) %= MOD;
          (cpp_assign(now, "*=", cpp_update(i, "--"))) %= MOD;
          (cpp_assign(now, "*=", ntt.inv((2 * cpp_update(cnt, "++"))))) %= MOD;
          v.push_back(now);
        }
      }
      pq.push(P(v.size(), memo.size()));
      memo.push_back(v);
    }
  }
  while ((pq.size() > 1))
  {
    var l = pq.top();
    var r: dynamic;
    pq.pop();
    r = pq.top();
    pq.pop();
    memo[l.second] = ntt.multiply(memo[l.second], memo[r.second]);
    pq.push(P(memo[l.second].size(), l.second));
  }
  var id = pq.top().second;
  var oddf = cpp_construct(2, 1);
  {
    var i = 3;
    while ((i <= (2 * n)))
    {
      var now = ((oddf.back() * i) % MOD);
      oddf.push_back(now);
      i += 2;
    }
  }
  var res = 0;
  var len = memo[id].size();
  {
    var i = 0;
    while ((i < len))
    {
      var now = ((memo[id][i] * oddf[(n - i)]) % MOD);
      if ((i & 1))
      {
        (cpp_assign(res, "+=", (MOD - now))) %= MOD;
      } else
      {
        (cpp_assign(res, "+=", now)) %= MOD;
      }
      i += 1;
    }
  }
  write(res, "\n");
  return 0;
}
