// Translated from solution.cpp.

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((b > a))
  {
    a = b;
  }
}

var INIT: dynamic = 0;

class segment_tree
{
  var n: dynamic = cpp_uninitialized();
  var dat: dynamic = cpp_uninitialized();
  func function(a: dynamic, b: dynamic) -> dynamic
  {
      return max(a, b);
    }
  func query(a: dynamic, b: dynamic, k: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      if (((r <= a) || (b <= l)))
      {
        return INIT;
      }
      if (((a <= l) && (r <= b)))
      {
        return dat[k];
      } else
      {
        var vl: dynamic = query(a, b, ((k * 2) + 1), l, (((l + r)) / 2));
        var vr: dynamic = query(a, b, ((k * 2) + 2), (((l + r)) / 2), r);
        return function(vl, vr);
      }
    }
  func segment_tree(n: dynamic) -> dynamic
  {
      n = 1;
      while ((n < n))
      {
        n *= 2;
      }
      dat.resize(((2 * n) - 1), INIT);
    }
  func update(k: dynamic, a: dynamic) -> dynamic
  {
      k += (n - 1);
      dat[k] = a;
      while ((k > 0))
      {
        k = (((k - 1)) / 2);
        dat[k] = function(dat[((k * 2) + 1)], dat[((k * 2) + 2)]);
      }
    }
  func query(a: dynamic, b: dynamic) -> dynamic
  {
      return query(a, b, 0, 0, n);
    }
}

func next(idx: dynamic, mod: dynamic) -> dynamic
{
  return (((idx + 1)) % mod);
}

func mod_sub(a: dynamic, b: dynamic, mod: dynamic) -> dynamic
{
  return ((((a - b) + mod)) % mod);
}

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      a[i] -= 1;
      i += 1;
    }
  }
  var number_of_edges: dynamic = (n / 2);
  var lds: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if (used[i])
      {
        i += 1;
        continue;
      }
      used[i] = true;
      used[a[i]] = true;
      var sequence: dynamic = cpp_construct(n, -1);
      var pos: dynamic = (a[i] - i);
      {
        var j: dynamic = (i + 1);
        while ((j != a[i]))
        {
          var pos_a: dynamic = mod_sub(a[j], i, n);
          if ((pos_a > pos))
          {
            sequence[((n - pos_a) - 1)] = mod_sub(j, i, n);
          }
          j = next(j, n);
        }
      }
      sequence.erase(remove(sequence.begin(), sequence.end(), -1), sequence.end());
      {
        var i: dynamic = 0;
        while ((i < static_cast(sequence.size())))
        {
          var value: dynamic = (seg.query((sequence[i] + 1), n) + 1);
          chmax(lds, value);
          seg.update(sequence[i], value);
          i += 1;
        }
      }
      i += 1;
    }
  }
  var ans: dynamic = min(((lds + 1) + k), number_of_edges);
  write(ans, "\n");
  return EXIT_SUCCESS;
}
