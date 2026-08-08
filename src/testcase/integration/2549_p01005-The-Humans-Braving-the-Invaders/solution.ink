// Translated from solution.cpp.

var Zero1 = cpp_construct(0);

var Zero2 = cpp_construct(make_pair(false, 0));

class Node
{
  var sum: dynamic;
  var lazy: dynamic;
  func Node()
  {
      this->sum = cpp_construct(Zero1);
      lazy = Zero2;
    }
}

class lazy_segtree
{
  var N: dynamic;
  var dat: dynamic;
  func lazy_segtree(n: dynamic)
  {
      this->N = cpp_construct(1);
      while ((N < n))
      {
        N *= 2;
      }
      dat.resize((2 * N));
    }
  func lazy_connect(l: dynamic, r: dynamic)
  {
      if ((l.first || r.first))
      {
        return make_pair(1, -1);
      } else
      {
        return make_pair(0, (l.second + r.second));
      }
    }
  func lazy_func(k: dynamic, a: dynamic, b: dynamic)
  {
      if (dat[k].lazy.first)
      {
        dat[k].sum = 0;
      } else
      {
        dat[k].sum += (dat[k].lazy.second * ((b - a)));
      }
    }
  func connect(l: dynamic, r: dynamic)
  {
      return (l + r);
    }
  func lazy_evaluate_node(k: dynamic, a: dynamic, b: dynamic)
  {
      lazy_func(k, a, b);
      if ((k < N))
      {
        dat[(2 * k)].lazy = lazy_connect(dat[(2 * k)].lazy, dat[k].lazy);
        dat[((2 * k) + 1)].lazy = lazy_connect(dat[((2 * k) + 1)].lazy, dat[k].lazy);
      }
      dat[k].lazy = Zero2;
    }
  func update_node(k: dynamic)
  {
      dat[k].sum = connect(dat[(2 * k)].sum, dat[((2 * k) + 1)].sum);
    }
  func update(l: dynamic, r: dynamic, v: dynamic, k: dynamic = 1, a: dynamic = 0, b: dynamic = -1)
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
      var m = (((a + b)) / 2);
      update(l, r, v, (2 * k), a, m);
      update(l, r, v, ((2 * k) + 1), m, b);
      update_node(k);
    }
  func get(l: dynamic, r: dynamic, k: dynamic = 1, a: dynamic = 0, b: dynamic = -1)
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
      var m = (((a + b)) / 2);
      var vl = get(l, r, (2 * k), a, m);
      var vr = get(l, r, ((2 * k) + 1), m, b);
      update_node(k);
      return connect(vl, vr);
    }
}

class query
{
  var type_cpp: dynamic;
  var a: dynamic;
  var b: dynamic;
}

class Compress
{
  var mp: dynamic;
  var revmp: dynamic;
  func Compress(vs: dynamic)
  {
      setmp(vs);
    }
  func Compress()
  {
      this->mp = cpp_construct();
      this->revmp = cpp_construct();
    }
  func setmp(vs: dynamic)
  {
      sort(vs.begin(), vs.end());
      vs.erase(unique(vs.begin(), vs.end()), vs.end());
      {
        var i = 0;
        while ((i < static_cast(vs.size())))
        {
          mp[vs[i]] = i;
          revmp[i] = vs[i];
          i += 1;
        }
      }
    }
}

func main()
{
  var Q: dynamic;
  while (cpp_comma((cin >> Q), Q))
  {
    var L: dynamic;
    read(L);
    var cp: dynamic;
    var qs: dynamic;
    {
      var xs: dynamic;
      xs.push_back(0);
      var nowdis = 0;
      {
        var i = 0;
        while ((i < Q))
        {
          var a: dynamic;
          read(a);
          if ((a == 0))
          {
            xs.push_back((nowdis + L));
            qs.push_back([a, -1, -1]);
          } else if ((a == 1))
          {
            var d: dynamic;
            read(d);
            nowdis += d;
            xs.push_back(nowdis);
            qs.push_back([a, d, -1]);
          } else if ((a == 2))
          {
            var k: dynamic;
            read(k);
            qs.push_back([a, k, -1]);
          } else if ((a == 3))
          {
            var x: dynamic;
            var r: dynamic;
            read(x, r);
            xs.push_back(((nowdis + x) + r));
            xs.push_back(((nowdis + x) - r));
            qs.push_back([a, ((nowdis + x) - r), ((nowdis + x) + r)]);
          } else if ((a == 4))
          {
            var k: dynamic;
            read(k);
            qs.push_back([a, k, -1]);
          }
          i += 1;
        }
      }
      cp.setmp(xs);
    }
    var seg = cpp_construct(cp.mp.size());
    var nowx = 0;
    for (var q in qs)
    {
      var __cpp_switch_1 = q.type_cpp;
      if (__cpp_switch_1 == 0)
      {
        seg.update(cp.mp[(nowx + L)], (cp.mp[(nowx + L)] + 1), make_pair(0, 1));
        break;
      }
      else if (__cpp_switch_1 == 1)
      {
        {
        var damage = seg.get((cp.mp[nowx] + 1), (cp.mp[(nowx + q.a)] + 1));
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
        var amin = -1;
        var amax = 3e5;
        while (((amin + 1) != amax))
        {
        var amid = cpp_construct((((amin + amax)) / 2));
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
        var bomb = seg.get(cp.mp[q.a], (cp.mp[q.b] + 1));
        write("bomb ", bomb, "\n");
        seg.update(cp.mp[q.a], (cp.mp[q.b] + 1), make_pair(1, 0));
        }
        break;
      }
      else if (__cpp_switch_1 == 4)
      {
        {
        var amin = -1;
        var amax = 3e5;
        while (((amin + 1) != amax))
        {
        var amid = cpp_construct((((amin + amax)) / 2));
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
        var dis = (cp.revmp[((cp.mp[nowx] + 1) + amax)] - nowx);
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
