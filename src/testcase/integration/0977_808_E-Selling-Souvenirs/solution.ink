// Translated from solution.cpp.

var N = 1e5;

var w = cpp_array(N);

var c = cpp_array(N);

var items: dynamic;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      read(w[i], c[i]);
      items.push_back(make_pair(((-6 * c[i]) / w[i]), i));
      i += 1;
    }
  }
  sort(items.begin(), items.end());
  var vals = cpp_array(10);
  {
    var i = 0;
    while ((i < 10))
    {
      vals[i] = 0;
      i += 1;
    }
  }
  var base = 0;
  var high = 0;
  {
    var i = 0;
    while ((i < cpp_cast(items.size())))
    {
      var ind = items[i].second;
      {
        var j = (9 - w[ind]);
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
        var dist = max(min((high - 5), ((m - base) - 9)), 0);
        base += dist;
        high -= dist;
        {
          var j = 0;
          while ((j < dist))
          {
            vals[j] = 0;
            j += 1;
          }
        }
        {
          var j = dist;
          while ((j < 10))
          {
            var tmp = vals[j];
            vals[j] = 0;
            vals[(j - dist)] = tmp;
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  var max = 0;
  {
    var i = base;
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
