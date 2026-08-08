// Translated from solution.cpp.

var BIG_NUM = cpp_expression("#include<b");

var HUGE_NUM = cpp_expression("#include<bits/std");

var MOD = cpp_expression("#include<b");

var EPS = cpp_expression("#include<bi");

var NUM = cpp_expression("#includ");

class Info
{
  func Info(arg_x: dynamic, arg_y: dynamic, arg_z: dynamic)
  {
      x = arg_x;
      y = arg_y;
      z = arg_z;
    }
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
}

var N: dynamic;

var boss = cpp_array(NUM);

var height = cpp_array(NUM);

var diff_x = [-1, 0, 1];

var diff_y = [-1, 0, 1];

var diff_z = [-1, 0, 1];

var X: dynamic;

var Y: dynamic;

var Z: dynamic;

var num_DEL: dynamic;

var num_REMAIN: dynamic;

var info: dynamic;

var DEL_GROUP = cpp_array(NUM);

var info_DELETE = cpp_array(NUM);

var info_REMAIN = cpp_array(NUM);

var DELETE = cpp_array(NUM);

var REMAIN = cpp_array(NUM);

func rangeCheck(x: dynamic, y: dynamic, z: dynamic)
{
  return ((((((x >= 0) && (x <= (X - 1))) && (y >= 0)) && (y <= (Y - 1))) && (z >= 0)) && (z <= (Z - 1)));
}

func get_boss(id: dynamic)
{
  if ((boss[id] == id))
  {
    return id;
  } else
  {
    return cpp_assign(boss[id], "=", get_boss(boss[id]));
  }
}

func is_same(x: dynamic, y: dynamic)
{
  return (get_boss(x) == get_boss(y));
}

func unite(x: dynamic, y: dynamic)
{
  var boss_x = get_boss(x);
  var boss_y = get_boss(y);
  if ((boss_x == boss_y))
  {
    return;
  }
  if ((height[x] > height[y]))
  {
    boss[boss_y] = boss_x;
  } else if ((height[x] < height[y]))
  {
    boss[boss_x] = boss_y;
  } else
  {
    boss[boss_y] = boss_x;
    height[x] += 1;
  }
}

func init(num: dynamic)
{
  {
    var i = 0;
    while ((i < num))
    {
      boss[i] = i;
      height[i] = 0;
      i += 1;
    }
  }
}

func is_DELETE(x: dynamic, y: dynamic, z: dynamic)
{
  var at = DELETE[x].find(make_pair(y, z));
  if ((at != DELETE[x].end()))
  {
    return true;
  } else
  {
    return false;
  }
}

func main()
{
  scanf("%d %d %d %d", (&X), (&Z), (&Y), (&num_DEL));
  var index_DELETE = 0;
  {
    var i = 0;
    while ((i < num_DEL))
    {
      var x: dynamic;
      var z: dynamic;
      var y: dynamic;
      scanf("%d %d %d", (&x), (&z), (&y));
      DELETE[x][P(y, z)] = cpp_update(index_DELETE, "++");
      info_DELETE[x].push_back(P(y, z));
      info.push_back(Info(x, y, z));
      i += 1;
    }
  }
  init(index_DELETE);
  {
    var i = 0;
    while ((i < index_DELETE))
    {
      var tmp_info = info[i];
      {
        var a = 0;
        while ((a < 3))
        {
          {
            var b = 0;
            while ((b < 3))
            {
              {
                var c = 0;
                while ((c < 3))
                {
                  if ((((diff_x[a] == 0) && (diff_y[b] == 0)) && (diff_z[c] == 0)))
                  {
                    c += 1;
                    continue;
                  }
                  var adj_x = (tmp_info.x + diff_x[a]);
                  var adj_y = (tmp_info.y + diff_y[b]);
                  var adj_z = (tmp_info.z + diff_z[c]);
                  if (((rangeCheck(adj_x, adj_y, adj_z) == true) && (is_DELETE(adj_x, adj_y, adj_z) == true)))
                  {
                    var from_cpp = i;
                    var to = DELETE[adj_x][P(adj_y, adj_z)];
                    unite(from_cpp, to);
                  }
                  c += 1;
                }
              }
              b += 1;
            }
          }
          a += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < index_DELETE))
    {
      DEL_GROUP[get_boss(i)].push_back(Info(info[i]));
      i += 1;
    }
  }
  var ans = 1;
  {
    var i = 0;
    while ((i < index_DELETE))
    {
      if ((DEL_GROUP[i].size() == 0))
      {
        i += 1;
        continue;
      }
      var index_REMAIN = 0;
      var min_x = BIG_NUM;
      var max_x = (-BIG_NUM);
      {
        var k = 0;
        while ((k < DEL_GROUP[i].size()))
        {
          min_x = min(min_x, DEL_GROUP[i][k].x);
          max_x = max(max_x, DEL_GROUP[i][k].x);
          k += 1;
        }
      }
      min_x = max(0, (min_x - 2));
      max_x = min((X - 1), (max_x + 2));
      {
        var k = min_x;
        while ((k <= max_x))
        {
          REMAIN[k].clear();
          info_REMAIN[k].clear();
          k += 1;
        }
      }
      var L = BIG_NUM;
      var R = (-BIG_NUM);
      {
        var k = 0;
        while ((k < DEL_GROUP[i].size()))
        {
          var tmp_info = DEL_GROUP[i][k];
          {
            var a = 0;
            while ((a < 3))
            {
              {
                var b = 0;
                while ((b < 3))
                {
                  {
                    var c = 0;
                    while ((c < 3))
                    {
                      if ((((diff_x[a] == 0) && (diff_y[b] == 0)) && (diff_z[c] == 0)))
                      {
                        c += 1;
                        continue;
                      }
                      var adj_x = (tmp_info.x + diff_x[a]);
                      var adj_y = (tmp_info.y + diff_y[b]);
                      var adj_z = (tmp_info.z + diff_z[c]);
                      if (((rangeCheck(adj_x, adj_y, adj_z) == false) || (is_DELETE(adj_x, adj_y, adj_z) == true)))
                      {
                        c += 1;
                        continue;
                      }
                      var at = REMAIN[adj_x].find(P(adj_y, adj_z));
                      if ((at != REMAIN[adj_x].end()))
                      {
                        c += 1;
                        continue;
                      }
                      L = min(L, adj_x);
                      R = max(R, adj_x);
                      REMAIN[adj_x][P(adj_y, adj_z)] = cpp_update(index_REMAIN, "++");
                      info_REMAIN[adj_x].push_back(P(adj_y, adj_z));
                      c += 1;
                    }
                  }
                  b += 1;
                }
              }
              a += 1;
            }
          }
          k += 1;
        }
      }
      if ((index_REMAIN == 0))
      {
        i += 1;
        continue;
      }
      init(index_REMAIN);
      {
        var x = L;
        while ((x <= R))
        {
          if ((info_REMAIN[x].size() == 0))
          {
            x += 1;
            continue;
          }
          {
            var loop = 0;
            while ((loop < info_REMAIN[x].size()))
            {
              var tmp = info_REMAIN[x][loop];
              {
                var a = 0;
                while ((a < 3))
                {
                  {
                    var b = 0;
                    while ((b < 3))
                    {
                      {
                        var c = 0;
                        while ((c < 3))
                        {
                          var count_zero = 0;
                          if ((diff_x[a] == 0))
                          {
                            count_zero += 1;
                          }
                          if ((diff_y[b] == 0))
                          {
                            count_zero += 1;
                          }
                          if ((diff_z[c] == 0))
                          {
                            count_zero += 1;
                          }
                          if ((count_zero != 2))
                          {
                            c += 1;
                            continue;
                          }
                          var adj_x = (x + diff_x[a]);
                          var adj_y = (tmp.first + diff_y[b]);
                          var adj_z = (tmp.second + diff_z[c]);
                          if ((rangeCheck(adj_x, adj_y, adj_z) == true))
                          {
                            var at = REMAIN[adj_x].find(P(adj_y, adj_z));
                            if ((at == REMAIN[adj_x].end()))
                            {
                              c += 1;
                              continue;
                            }
                            var from_cpp = REMAIN[x][tmp];
                            var to = REMAIN[adj_x][P(adj_y, adj_z)];
                            unite(from_cpp, to);
                          }
                          c += 1;
                        }
                      }
                      b += 1;
                    }
                  }
                  a += 1;
                }
              }
              loop += 1;
            }
          }
          x += 1;
        }
      }
      var num_group = 0;
      {
        var k = 0;
        while ((k < index_REMAIN))
        {
          if ((k == get_boss(k)))
          {
            num_group += 1;
          }
          k += 1;
        }
      }
      ans += (num_group - 1);
      i += 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
