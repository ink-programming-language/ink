// Translated from solution.cpp.

func chmax(a: dynamic, b: dynamic)
{
  if ((b > a))
  {
    a = b;
  }
}

var INIT = 0;

class segment_tree
{
  var n: dynamic;
  var dat: dynamic;
  func function(a: dynamic, b: dynamic)
  {
      return max(a, b);
    }
  func query(a: dynamic, b: dynamic, k: dynamic, l: dynamic, r: dynamic)
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
        var vl = query(a, b, ((k * 2) + 1), l, (((l + r)) / 2));
        var vr = query(a, b, ((k * 2) + 2), (((l + r)) / 2), r);
        return function(vl, vr);
      }
    }
  func segment_tree(n: dynamic)
  {
      n = 1;
      while ((n < n))
      {
        n *= 2;
      }
      dat.resize(((2 * n) - 1), INIT);
    }
  func update(k: dynamic, a: dynamic)
  {
      k += (n - 1);
      dat[k] = a;
      while ((k > 0))
      {
        k = (((k - 1)) / 2);
        dat[k] = function(dat[((k * 2) + 1)], dat[((k * 2) + 2)]);
      }
    }
  func query(a: dynamic, b: dynamic)
  {
      return query(a, b, 0, 0, n);
    }
}

func next(idx: dynamic, mod: dynamic)
{
  return (((idx + 1)) % mod);
}

func mod_sub(a: dynamic, b: dynamic, mod: dynamic)
{
  return ((((a - b) + mod)) % mod);
}

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      a[i] -= 1;
      i += 1;
    }
  }
  var number_of_edges = (n / 2);
  var lds = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if (used[i])
      {
        i += 1;
        continue;
      }
      used[i] = true;
      used[a[i]] = true;
      var sequence = cpp_construct(n, -1);
      var pos = (a[i] - i);
      {
        var j = (i + 1);
        while ((j != a[i]))
        {
          var pos_a = mod_sub(a[j], i, n);
          if ((pos_a > pos))
          {
            sequence[((n - pos_a) - 1)] = mod_sub(j, i, n);
          }
          j = next(j, n);
        }
      }
      sequence.erase(remove(sequence.begin(), sequence.end(), -1), sequence.end());
      {
        var i = 0;
        while ((i < static_cast(sequence.size())))
        {
          var value = (seg.query((sequence[i] + 1), n) + 1);
          chmax(lds, value);
          seg.update(sequence[i], value);
          i += 1;
        }
      }
      i += 1;
    }
  }
  var ans = min(((lds + 1) + k), number_of_edges);
  write(ans, "\n");
  return EXIT_SUCCESS;
}
