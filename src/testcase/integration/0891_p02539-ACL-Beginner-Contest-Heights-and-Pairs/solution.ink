// Translated from solution.cpp.

var MOD: dynamic = cpp_expression("#include <bits/s");

class NTT
{
  var base: dynamic = cpp_uninitialized();
  var maxb: dynamic = cpp_uninitialized();
  var root: dynamic = cpp_uninitialized();
  var rv: dynamic = cpp_uninitialized();
  var roots: dynamic = cpp_uninitialized();
  var invr: dynamic = cpp_uninitialized();
  func NTT() -> dynamic
  {
      self->base = cpp_construct(1);
      self->rv = cpp_construct([0, 1]);
      self->roots = cpp_construct([0, 1]);
      self->invr = cpp_construct([0, 1]);
      assert(((mod >= 3) && (mod & 1)));
      var tmp: dynamic = (mod - 1);
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
  func mpow(x: dynamic, n: dynamic) -> dynamic
  {
      var res: dynamic = 1;
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
  func inv(x: dynamic) -> dynamic
  {
      return mpow(x, (mod - 2));
    }
  func add(x: dynamic, y: dynamic) -> dynamic
  {
      if (((cpp_assign(x, "+=", y)) >= mod))
      {
        x -= mod;
      }
      return x;
    }
  func mul(x: dynamic, y: dynamic) -> dynamic
  {
      return cpp_cast(((((1 * x) * y) % mod)));
    }
  func ensure_base(nb: dynamic) -> dynamic
  {
      if ((nb <= base))
      {
        return;
      }
      rv.resize((1 << nb));
      roots.resize((1 << nb));
      invr.resize((1 << nb));
      {
        var i: dynamic = 0;
        while ((i < ((1 << nb))))
        {
          rv[i] = (((rv[(i >> 1)] >> 1)) + ((((i & 1)) << ((nb - 1)))));
          i += 1;
        }
      }
      assert((nb <= maxb));
      while ((base < nb))
      {
        var z: dynamic = mpow(root, (1 << (((maxb - 1) - base))));
        var invz: dynamic = inv(z);
        {
          var i: dynamic = (1 << ((base - 1)));
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
  func ntt(a: dynamic, n: dynamic, sg: dynamic = 0) -> dynamic
  {
      assert((((n & ((n - 1)))) == 0));
      {
        var i: dynamic = 0;
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
        var k: dynamic = 1;
        while ((k < n))
        {
          {
            var i: dynamic = 0;
            while ((i < n))
            {
              {
                var j: dynamic = 0;
                while ((j < k))
                {
                  var z: dynamic = mul(a[((i + j) + k)], ( (sg) ? roots[(j + k)] : invr[(j + k)]));
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
      var invn: dynamic = inv(n);
      if (sg)
      {
        {
          var i: dynamic = 0;
          while ((i < n))
          {
            a[i] = mul(a[i], invn);
            i += 1;
          }
        }
      }
    }
  func multiply(a: dynamic, b: dynamic) -> dynamic
  {
      var need: dynamic = ((a.size() + b.size()) - 1);
      var nb: dynamic = 1;
      while ((((1 << nb)) < need))
      {
        nb += 1;
      }
      ensure_base(nb);
      var sz: dynamic = (1 << nb);
      var fa: dynamic = cpp_construct(sz, 0);
      var fb: dynamic = cpp_construct(sz, 0);
      {
        var i: dynamic = 0;
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
        var i: dynamic = 0;
        while ((i < sz))
        {
          fa[i] = mul(fa[i], fb[i]);
          i += 1;
        }
      }
      ntt(fa, sz, 1);
      {
        var i: dynamic = 0;
        while ((i < need))
        {
          res[i] = fa[i];
          i += 1;
        }
      }
      return res;
    }
}

var n: dynamic = cpp_uninitialized();

var memo: dynamic = cpp_uninitialized();

var pq: dynamic = cpp_uninitialized();

var ntt: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  {
    var mp: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < (2 * n)))
      {
        var x: dynamic = cpp_uninitialized();
        read(x);
        mp[x] += 1;
        i += 1;
      }
    }
    for (var __cpp_item_1: dynamic in mp)
    {
      var (cpp_name, p): dynamic = __cpp_item_1;
      var v: dynamic = cpp_construct(1, 1);
      var now: dynamic = 1;
      var cnt: dynamic = 1;
      {
        var i: dynamic = p;
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
    var l: dynamic = pq.top();
    var r: dynamic = cpp_uninitialized();
    pq.pop();
    r = pq.top();
    pq.pop();
    memo[l.second] = ntt.multiply(memo[l.second], memo[r.second]);
    pq.push(P(memo[l.second].size(), l.second));
  }
  var id: dynamic = pq.top().second;
  var oddf: dynamic = cpp_construct(2, 1);
  {
    var i: dynamic = 3;
    while ((i <= (2 * n)))
    {
      var now: dynamic = ((oddf.back() * i) % MOD);
      oddf.push_back(now);
      i += 2;
    }
  }
  var res: dynamic = 0;
  var len: dynamic = memo[id].size();
  {
    var i: dynamic = 0;
    while ((i < len))
    {
      var now: dynamic = ((memo[id][i] * oddf[(n - i)]) % MOD);
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
