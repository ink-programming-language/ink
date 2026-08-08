// Translated from solution.cpp.

var MAXN = (1e6 + 10);

var INF = (1e9 + 10);

var Mod = (1e9 + 7);

class node
{
  var val: dynamic;
  var lazy: dynamic;
  var zero: dynamic;
  func node()
  {
      val = 0;
      lazy = 0;
      zero = false;
    }
}

var segt_2 = cpp_array((4 * MAXN));

var query = cpp_array(MAXN);

var segt_1 = cpp_array((4 * MAXN));

func set_update(id: dynamic, l: dynamic, r: dynamic, p: dynamic, d: dynamic)
{
  if (((l > p) || (r < p)))
  {
    return;
  }
  if ((l == r))
  {
    segt_1[id] = d;
    return;
  }
  var mid = (((l + r)) / 2);
  set_update(((2 * id) + 1), l, mid, p, d);
  set_update(((2 * id) + 2), (mid + 1), r, p, d);
  segt_1[id] = min(segt_1[((2 * id) + 1)], segt_1[((2 * id) + 2)]);
}

func get_min(id: dynamic, L: dynamic, R: dynamic, l: dynamic, r: dynamic)
{
  if (((L > r) || (R < l)))
  {
    return INF;
  }
  if (((L >= l) && (R <= r)))
  {
    return segt_1[id];
  }
  var mid = (((L + R)) / 2);
  return min(get_min(((2 * id) + 1), L, mid, l, r), get_min(((2 * id) + 2), (mid + 1), R, l, r));
}

func lzu(id: dynamic)
{
  if (segt_2[id].zero)
  {
    segt_2[((2 * id) + 1)].val = 0;
    segt_2[((2 * id) + 1)].lazy = 0;
    segt_2[((2 * id) + 2)].val = 0;
    segt_2[((2 * id) + 2)].lazy = 0;
    segt_2[((2 * id) + 1)].zero = true;
    segt_2[((2 * id) + 2)].zero = true;
  }
  segt_2[((2 * id) + 1)].val += segt_2[id].lazy;
  segt_2[((2 * id) + 1)].val %= Mod;
  segt_2[((2 * id) + 2)].val += segt_2[id].lazy;
  segt_2[((2 * id) + 2)].val %= Mod;
  segt_2[((2 * id) + 1)].lazy += segt_2[id].lazy;
  segt_2[((2 * id) + 1)].lazy %= Mod;
  segt_2[((2 * id) + 2)].lazy += segt_2[id].lazy;
  segt_2[((2 * id) + 2)].lazy %= Mod;
  segt_2[id].lazy = 0;
  segt_2[id].zero = false;
}

func add_update(id: dynamic, L: dynamic, R: dynamic, l: dynamic, r: dynamic, d: dynamic)
{
  if ((((l > r) || (L > r)) || (R < l)))
  {
    return;
  }
  if (((L >= l) && (R <= r)))
  {
    segt_2[id].val += d;
    segt_2[id].val %= Mod;
    segt_2[id].lazy += d;
    segt_2[id].lazy %= Mod;
    return;
  }
  lzu(id);
  var mid = (((L + R)) / 2);
  add_update(((2 * id) + 1), L, mid, l, r, d);
  add_update(((2 * id) + 2), (mid + 1), R, l, r, d);
  segt_2[id].val = (segt_2[((2 * id) + 1)].val + segt_2[((2 * id) + 2)].val);
}

func zero_update(id: dynamic, L: dynamic, R: dynamic, l: dynamic, r: dynamic)
{
  if ((((l > r) || (L > r)) || (R < l)))
  {
    return;
  }
  if (((L >= l) && (R <= r)))
  {
    segt_2[id].val = 0;
    segt_2[id].lazy = 0;
    segt_2[id].zero = true;
    return;
  }
  lzu(id);
  var mid = (((L + R)) / 2);
  zero_update(((2 * id) + 1), L, mid, l, r);
  zero_update(((2 * id) + 2), (mid + 1), R, l, r);
  segt_2[id].val = (segt_2[((2 * id) + 1)].val + segt_2[((2 * id) + 2)].val);
}

func get_res(id: dynamic, l: dynamic, r: dynamic, p: dynamic)
{
  if (((l > p) || (r < p)))
  {
    return 0;
  }
  if ((l == r))
  {
    return segt_2[id].val;
  }
  lzu(id);
  var mid = (((l + r)) / 2);
  return (get_res(((2 * id) + 1), l, mid, p) + get_res(((2 * id) + 2), (mid + 1), r, p));
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  read(n, m, k);
  {
    var i = 0;
    while ((i < k))
    {
      var x1: dynamic;
      var y1: dynamic;
      var x2: dynamic;
      var y2: dynamic;
      read(x1, y1, x2, y2);
      query[x1].push_back(make_pair(1, make_pair(y1, y2)));
      query[(x2 + 1)].push_back(make_pair(0, make_pair(y1, y2)));
      i += 1;
    }
  }
  set_update(0, 0, (m + 1), (m + 1), (m + 1));
  {
    var i = 0;
    while ((i <= m))
    {
      set_update(0, 0, (m + 1), i, INF);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      sort(query[i].begin(), query[i].end());
      {
        var j = 0;
        while ((j < query[i].size()))
        {
          if (query[i][j].first)
          {
            set_update(0, 0, (m + 1), query[i][j].second.second, query[i][j].second.first);
          } else
          {
            set_update(0, 0, (m + 1), query[i][j].second.second, INF);
          }
          j += 1;
        }
      }
      if ((i == 1))
      {
        add_update(0, 0, (m + 1), 1, (get_min(0, 0, (m + 1), 0, (m + 1)) - 1), 1);
      }
      {
        var j = (cpp_cast((query[i].size())) - 1);
        while ((j >= 0))
        {
          if (query[i][j].first)
          {
            zero_update(0, 0, (m + 1), query[i][j].second.first, query[i][j].second.second);
          } else
          {
            add_update(0, 0, (m + 1), query[i][j].second.first, (get_min(0, 0, (m + 1), (query[i][j].second.first - 1), (m + 1)) - 1), get_res(0, 0, (m + 1), (query[i][j].second.first - 1)));
          }
          j -= 1;
        }
      }
      i += 1;
    }
  }
  write(get_res(0, 0, (m + 1), m));
}
