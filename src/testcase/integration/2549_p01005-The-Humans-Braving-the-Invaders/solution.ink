// Translated from solution.cpp.

var Zero1: dynamic = cpp_construct(0);

var Zero2: dynamic = cpp_construct(make_pair(false, 0));

class Node
{
  var sum: dynamic = cpp_uninitialized();
  var lazy: dynamic = cpp_uninitialized();
  func Node() -> dynamic
  {
      self->sum = cpp_construct(Zero1);
      lazy = Zero2;
    }
}

class lazy_segtree
{
  var N: dynamic = cpp_uninitialized();
  var dat: dynamic = cpp_uninitialized();
  func lazy_segtree(n: dynamic) -> dynamic
  {
      self->N = cpp_construct(1);
      while ((N < n))
      {
        N *= 2;
      }
      dat.resize((2 * N));
    }
  func lazy_connect(l: dynamic, r: dynamic) -> dynamic
  {
      if ((l.first || r.first))
      {
        return make_pair(1, -1);
      } else
      {
        return make_pair(0, (l.second + r.second));
      }
    }
  func lazy_func(k: dynamic, a: dynamic, b: dynamic) -> dynamic
  {
      if (dat[k].lazy.first)
      {
        dat[k].sum = 0;
      } else
      {
        dat[k].sum += (dat[k].lazy.second * ((b - a)));
      }
    }
  func connect(l: dynamic, r: dynamic) -> dynamic
  {
      return (l + r);
    }
  func lazy_evaluate_node(k: dynamic, a: dynamic, b: dynamic) -> dynamic
  {
      lazy_func(k, a, b);
      if ((k < N))
      {
        dat[(2 * k)].lazy = lazy_connect(dat[(2 * k)].lazy, dat[k].lazy);
        dat[((2 * k) + 1)].lazy = lazy_connect(dat[((2 * k) + 1)].lazy, dat[k].lazy);
      }
      dat[k].lazy = Zero2;
    }
  func update_node(k: dynamic) -> dynamic
  {
      dat[k].sum = connect(dat[(2 * k)].sum, dat[((2 * k) + 1)].sum);
    }
  func update(l: dynamic, r: dynamic, v: dynamic, k: dynamic = 1, a: dynamic = 0, b: dynamic = -1) -> dynamic
  {
      if ((b == -1))
      {
        b = N;
      }
      if (((l < 0) || (r < 0)))
      {
        assert(false);
      }
      lazy_evaluate_node(k, a, b);
      if (((b <= l) || (r <= a)))
      {
        return;
      }
      if (((l <= a) && (b <= r)))
      {
        dat[k].lazy = lazy_connect(dat[k].lazy, v);
        lazy_evaluate_node(k, a, b);
        return;
      }
      var m: dynamic = (((a + b)) / 2);
      update(l, r, v, (2 * k), a, m);
      update(l, r, v, ((2 * k) + 1), m, b);
      update_node(k);
    }
  func get(l: dynamic, r: dynamic, k: dynamic = 1, a: dynamic = 0, b: dynamic = -1) -> dynamic
  {
      if ((b == -1))
      {
        b = N;
      }
      if (((l < 0) || (r < 0)))
      {
        assert(false);
      }
      lazy_evaluate_node(k, a, b);
      if (((b <= l) || (r <= a)))
      {
        return Zero1;
      }
      if (((l <= a) && (b <= r)))
      {
        return dat[k].sum;
      }
      var m: dynamic = (((a + b)) / 2);
      var vl: dynamic = get(l, r, (2 * k), a, m);
      var vr: dynamic = get(l, r, ((2 * k) + 1), m, b);
      update_node(k);
      return connect(vl, vr);
    }
}

class query
{
  var type_cpp: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
}

class Compress
{
  var mp: dynamic = cpp_uninitialized();
  var revmp: dynamic = cpp_uninitialized();
  func Compress(vs: dynamic) -> dynamic
  {
      setmp(vs);
    }
  func Compress() -> dynamic
  {
      self->mp = cpp_construct();
      self->revmp = cpp_construct();
    }
  func setmp(vs: dynamic) -> dynamic
  {
      sort(vs.begin(), vs.end());
      vs.erase(unique(vs.begin(), vs.end()), vs.end());
      {
        var i: dynamic = 0;
        while ((i < static_cast(vs.size())))
        {
          mp[vs[i]] = i;
          revmp[i] = vs[i];
          i += 1;
        }
      }
    }
}

func main() -> dynamic
{
  var Q: dynamic = cpp_uninitialized();
  while (cpp_comma((cin >> Q), Q))
  {
    var L: dynamic = cpp_uninitialized();
    read(L);
    var cp: dynamic = cpp_uninitialized();
    var qs: dynamic = cpp_uninitialized();
    {
      var xs: dynamic = cpp_uninitialized();
      xs.push_back(0);
      var nowdis: dynamic = 0;
      {
        var i: dynamic = 0;
        while ((i < Q))
        {
          var a: dynamic = cpp_uninitialized();
          read(a);
          if ((a == 0))
          {
            xs.push_back((nowdis + L));
            qs.push_back([a, -1, -1]);
          } else if ((a == 1))
          {
            var d: dynamic = cpp_uninitialized();
            read(d);
            nowdis += d;
            xs.push_back(nowdis);
            qs.push_back([a, d, -1]);
          } else if ((a == 2))
          {
            var k: dynamic = cpp_uninitialized();
            read(k);
            qs.push_back([a, k, -1]);
          } else if ((a == 3))
          {
            var x: dynamic = cpp_uninitialized();
            var r: dynamic = cpp_uninitialized();
            read(x, r);
            xs.push_back(((nowdis + x) + r));
            xs.push_back(((nowdis + x) - r));
            qs.push_back([a, ((nowdis + x) - r), ((nowdis + x) + r)]);
          } else if ((a == 4))
          {
            var k: dynamic = cpp_uninitialized();
            read(k);
            qs.push_back([a, k, -1]);
          }
          i += 1;
        }
      }
      cp.setmp(xs);
    }
    var seg: dynamic = cpp_construct(cp.mp.size());
    var nowx: dynamic = 0;
    for (var q: dynamic in qs)
    {
      var __cpp_switch_1: dynamic = q.type_cpp;
      if (__cpp_switch_1 == 0)
      {
        seg.update(cp.mp[(nowx + L)], (cp.mp[(nowx + L)] + 1), make_pair(0, 1));
        break;
      }
      else if (__cpp_switch_1 == 1)
      {
        {
        var damage: dynamic = seg.get((cp.mp[nowx] + 1), (cp.mp[(nowx + q.a)] + 1));
        seg.update(cp.mp[nowx], (cp.mp[(nowx + q.a)] + 1), make_pair(1, 0));
        if (damage)
        {
        write("damage", " ", damage, "\n");
        }
        nowx += q.a;
        }
        break;
      }
      else if (__cpp_switch_1 == 2)
      {
        {
        var amin: dynamic = -1;
        var amax: dynamic = 3e5;
        while (((amin + 1) != amax))
        {
        var amid: dynamic = cpp_construct((((amin + amax)) / 2));
        if ((seg.get((cp.mp[nowx] + 1), ((cp.mp[nowx] + 2) + amid)) >= q.a))
        {
        amax = amid;
        } else
        {
        amin = amid;
        }
        }
        if ((amax != 3e5))
        {
        write("hit", "\n");
        seg.update(((cp.mp[nowx] + 1) + amax), ((cp.mp[nowx] + 2) + amax), make_pair(0, -1));
        } else
        {
        write("miss", "\n");
        }
        }
        break;
      }
      else if (__cpp_switch_1 == 3)
      {
        {
        var bomb: dynamic = seg.get(cp.mp[q.a], (cp.mp[q.b] + 1));
        write("bomb ", bomb, "\n");
        seg.update(cp.mp[q.a], (cp.mp[q.b] + 1), make_pair(1, 0));
        }
        break;
      }
      else if (__cpp_switch_1 == 4)
      {
        {
        var amin: dynamic = -1;
        var amax: dynamic = 3e5;
        while (((amin + 1) != amax))
        {
        var amid: dynamic = cpp_construct((((amin + amax)) / 2));
        if ((seg.get((cp.mp[nowx] + 1), ((cp.mp[nowx] + 2) + amid)) >= q.a))
        {
        amax = amid;
        } else
        {
        amin = amid;
        }
        }
        if ((amax != 3e5))
        {
        var dis: dynamic = (cp.revmp[((cp.mp[nowx] + 1) + amax)] - nowx);
        write("distance ", dis, "\n");
        } else
        {
        write("distance -1", "\n");
        }
        }
        break;
      }
      else
      {
        assert(false);
        break;
      }
    }
    write("end", "\n");
    assert((cp.mp.size() == cp.revmp.size()));
  }
  return 0;
}
