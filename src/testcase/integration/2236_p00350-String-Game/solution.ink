// Translated from solution.cpp.

var mod: dynamic = (1e9 + 7);

var power: dynamic = cpp_array(234567);

var lad: dynamic = cpp_array(234567, 26);

class SegmentTree
{
  var hashed: dynamic = cpp_uninitialized();
  var lazy: dynamic = cpp_uninitialized();
  var sz: dynamic = cpp_uninitialized();
  func SegmentTree(n: dynamic) -> dynamic
  {
      sz = 1;
      while ((sz < n))
      {
        sz <<= 1;
      }
      hashed.assign(((2 * sz) - 1), 0);
      lazy.assign(((2 * sz) - 1), -1);
    }
  func push(k: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      if ((~lazy[k]))
      {
        hashed[k] = lad[lazy[k]][(r - l)];
        if ((k < (sz - 1)))
        {
          lazy[((2 * k) + 1)] = lazy[k];
          lazy[((2 * k) + 2)] = lazy[k];
        }
        lazy[k] = -1;
      }
    }
  func add(a: dynamic, b: dynamic, x: dynamic, k: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      push(k, l, r);
      if (((a >= r) || (b <= l)))
      {
        return;
      }
      if (((a <= l) && (r <= b)))
      {
        lazy[k] = x;
        push(k, l, r);
      } else
      {
        add(a, b, x, ((2 * k) + 1), l, (((l + r)) >> 1));
        add(a, b, x, ((2 * k) + 2), (((l + r)) >> 1), r);
        hashed[k] = ((hashed[((2 * k) + 1)] * power[(((r - l)) >> 1)]) + hashed[((2 * k) + 2)]);
      }
    }
  func add(a: dynamic, b: dynamic, x: dynamic) -> dynamic
  {
      add(a, b, x, 0, 0, sz);
    }
  func get(a: dynamic, b: dynamic, k: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      push(k, l, r);
      if (((a >= r) || (b <= l)))
      {
        return [0, 0];
      }
      if (((a <= l) & (r <= b)))
      {
        return [hashed[k], (r - l)];
      }
      var ll: dynamic = get(a, b, ((2 * k) + 1), l, (((l + r)) >> 1));
      var rr: dynamic = get(a, b, ((2 * k) + 2), (((l + r)) >> 1), r);
      return [((ll.first * power[rr.second]) + rr.first), (ll.second + rr.second)];
    }
  func get(a: dynamic, b: dynamic) -> dynamic
  {
      return (get(a, b, 0, 0, sz).first);
    }
}

func main() -> dynamic
{
  power[0] = 1;
  {
    var i: dynamic = 0;
    while ((i < 234566))
    {
      power[(i + 1)] = (power[i] * mod);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      var cc: dynamic = (cpp_char("a") + i);
      {
        var j: dynamic = 0;
        while ((j < 234566))
        {
          lad[i][(j + 1)] = ((lad[i][j] * mod) + cc);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var N: dynamic = cpp_uninitialized();
  var Q: dynamic = cpp_uninitialized();
  var U: dynamic = cpp_uninitialized();
  read(N);
  read(U);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      tree.add(i, (i + 1), (U[i] - cpp_char("a")));
      i += 1;
    }
  }
  read(Q);
  while (cpp_update(Q, "--"))
  {
    var S: dynamic = cpp_uninitialized();
    read(S);
    if ((S == "set"))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      var z: dynamic = cpp_uninitialized();
      read(x, y, z);
      tree.add(cpp_update(x, "--"), y, (z - cpp_char("a")));
    } else
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      var c: dynamic = cpp_uninitialized();
      var d: dynamic = cpp_uninitialized();
      read(a, b, c, d);
      a -= 1;
      c -= 1;
      var ok: dynamic = 0;
      var ng: dynamic = (min((d - c), (b - a)) + 1);
      while (((ng - ok) > 1))
      {
        var mid: dynamic = (((ok + ng)) >> 1);
        if ((tree.get(a, (a + mid)) == tree.get(c, (c + mid))))
        {
          ok = mid;
        } else
        {
          ng = mid;
        }
      }
      if (((ok == (d - c)) && (ok == (b - a))))
      {
        write("e\n");
      } else if ((((b - a) == ok) || ((((d - c) != ok) && (tree.get((a + ok), ((a + ok) + 1)) < tree.get((c + ok), ((c + ok) + 1)))))))
      {
        write("s\n");
      } else
      {
        write("t\n");
      }
    }
  }
}
