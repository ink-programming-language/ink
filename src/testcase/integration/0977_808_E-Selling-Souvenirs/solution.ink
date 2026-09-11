// Translated from solution.cpp.

var N: dynamic = 1e5;

var w: dynamic = cpp_array(N);

var c: dynamic = cpp_array(N);

var items: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(w[i], c[i]);
      items.push_back(make_pair(((-6 * c[i]) / w[i]), i));
      i += 1;
    }
  }
  sort(items.begin(), items.end());
  var vals: dynamic = cpp_array(10);
  {
    var i: dynamic = 0;
    while ((i < 10))
    {
      vals[i] = 0;
      i += 1;
    }
  }
  var base: dynamic = 0;
  var high: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(items.size())))
    {
      var ind: dynamic = items[i].second;
      {
        var j: dynamic = (9 - w[ind]);
        while ((j >= 0))
        {
          if (vals[j])
          {
            vals[(j + w[ind])] = max(vals[(j + w[ind])], (vals[j] + c[ind]));
          }
          j -= 1;
        }
      }
      if ((base == 0))
      {
        vals[w[ind]] = max(vals[w[ind]], (vals[0] + c[ind]));
      }
      high += w[ind];
      if ((high > 5))
      {
        var dist: dynamic = max(min((high - 5), ((m - base) - 9)), 0);
        base += dist;
        high -= dist;
        {
          var j: dynamic = 0;
          while ((j < dist))
          {
            vals[j] = 0;
            j += 1;
          }
        }
        {
          var j: dynamic = dist;
          while ((j < 10))
          {
            var tmp: dynamic = vals[j];
            vals[j] = 0;
            vals[(j - dist)] = tmp;
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  var max: dynamic = 0;
  {
    var i: dynamic = base;
    while ((((i <= m)) && ((i < (base + 10)))))
    {
      if ((vals[(i - base)] > max))
      {
        max = vals[(i - base)];
      }
      i += 1;
    }
  }
  write(max, cpp_char("\n"));
}
