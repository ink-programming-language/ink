// Translated from solution.cpp.

var BIG_NUM: dynamic = cpp_expression("#include<b");

var HUGE_NUM: dynamic = cpp_expression("#include<bits/std");

var MOD: dynamic = cpp_expression("#include<b");

var EPS: dynamic = cpp_expression("#include<bi");

var NUM: dynamic = cpp_expression("#includ");

class Info
{
  func Info(arg_x: dynamic, arg_y: dynamic, arg_z: dynamic) -> dynamic
  {
      x = arg_x;
      y = arg_y;
      z = arg_z;
    }
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
}

var N: dynamic = cpp_uninitialized();

var boss: dynamic = cpp_array(NUM);

var height: dynamic = cpp_array(NUM);

var diff_x: dynamic = [-1, 0, 1];

var diff_y: dynamic = [-1, 0, 1];

var diff_z: dynamic = [-1, 0, 1];

var X: dynamic = cpp_uninitialized();

var Y: dynamic = cpp_uninitialized();

var Z: dynamic = cpp_uninitialized();

var num_DEL: dynamic = cpp_uninitialized();

var num_REMAIN: dynamic = cpp_uninitialized();

var info: dynamic = cpp_uninitialized();

var DEL_GROUP: dynamic = cpp_array(NUM);

var info_DELETE: dynamic = cpp_array(NUM);

var info_REMAIN: dynamic = cpp_array(NUM);

var DELETE: dynamic = cpp_array(NUM);

var REMAIN: dynamic = cpp_array(NUM);

func rangeCheck(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  return ((((((x >= 0) && (x <= (X - 1))) && (y >= 0)) && (y <= (Y - 1))) && (z >= 0)) && (z <= (Z - 1)));
}

func get_boss(id: dynamic) -> dynamic
{
  if ((boss[id] == id))
  {
    return id;
  } else
  {
    return cpp_assign(boss[id], "=", get_boss(boss[id]));
  }
}

func is_same(x: dynamic, y: dynamic) -> dynamic
{
  return (get_boss(x) == get_boss(y));
}

func unite(x: dynamic, y: dynamic) -> dynamic
{
  var boss_x: dynamic = get_boss(x);
  var boss_y: dynamic = get_boss(y);
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

func init(num: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < num))
    {
      boss[i] = i;
      height[i] = 0;
      i += 1;
    }
  }
}

func is_DELETE(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  var at: dynamic = DELETE[x].find(make_pair(y, z));
  if ((at != DELETE[x].end()))
  {
    return true;
  } else
  {
    return false;
  }
}

func main() -> dynamic
{
  scanf("%d %d %d %d", (&X), (&Z), (&Y), (&num_DEL));
  var index_DELETE: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < num_DEL))
    {
      var x: dynamic = cpp_uninitialized();
      var z: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      scanf("%d %d %d", (&x), (&z), (&y));
      DELETE[x][P(y, z)] = cpp_update(index_DELETE, "++");
      info_DELETE[x].push_back(P(y, z));
      info.push_back(Info(x, y, z));
      i += 1;
    }
  }
  init(index_DELETE);
  {
    var i: dynamic = 0;
    while ((i < index_DELETE))
    {
      var tmp_info: dynamic = info[i];
      {
        var a: dynamic = 0;
        while ((a < 3))
        {
          {
            var b: dynamic = 0;
            while ((b < 3))
            {
              {
                var c: dynamic = 0;
                while ((c < 3))
                {
                  if ((((diff_x[a] == 0) && (diff_y[b] == 0)) && (diff_z[c] == 0)))
                  {
                    c += 1;
                    continue;
                  }
                  var adj_x: dynamic = (tmp_info.x + diff_x[a]);
                  var adj_y: dynamic = (tmp_info.y + diff_y[b]);
                  var adj_z: dynamic = (tmp_info.z + diff_z[c]);
                  if (((rangeCheck(adj_x, adj_y, adj_z) == true) && (is_DELETE(adj_x, adj_y, adj_z) == true)))
                  {
                    var from_cpp: dynamic = i;
                    var to: dynamic = DELETE[adj_x][P(adj_y, adj_z)];
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
    var i: dynamic = 0;
    while ((i < index_DELETE))
    {
      DEL_GROUP[get_boss(i)].push_back(Info(info[i]));
      i += 1;
    }
  }
  var ans: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < index_DELETE))
    {
      if ((DEL_GROUP[i].size() == 0))
      {
        i += 1;
        continue;
      }
      var index_REMAIN: dynamic = 0;
      var min_x: dynamic = BIG_NUM;
      var max_x: dynamic = (-BIG_NUM);
      {
        var k: dynamic = 0;
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
        var k: dynamic = min_x;
        while ((k <= max_x))
        {
          REMAIN[k].clear();
          info_REMAIN[k].clear();
          k += 1;
        }
      }
      var L: dynamic = BIG_NUM;
      var R: dynamic = (-BIG_NUM);
      {
        var k: dynamic = 0;
        while ((k < DEL_GROUP[i].size()))
        {
          var tmp_info: dynamic = DEL_GROUP[i][k];
          {
            var a: dynamic = 0;
            while ((a < 3))
            {
              {
                var b: dynamic = 0;
                while ((b < 3))
                {
                  {
                    var c: dynamic = 0;
                    while ((c < 3))
                    {
                      if ((((diff_x[a] == 0) && (diff_y[b] == 0)) && (diff_z[c] == 0)))
                      {
                        c += 1;
                        continue;
                      }
                      var adj_x: dynamic = (tmp_info.x + diff_x[a]);
                      var adj_y: dynamic = (tmp_info.y + diff_y[b]);
                      var adj_z: dynamic = (tmp_info.z + diff_z[c]);
                      if (((rangeCheck(adj_x, adj_y, adj_z) == false) || (is_DELETE(adj_x, adj_y, adj_z) == true)))
                      {
                        c += 1;
                        continue;
                      }
                      var at: dynamic = REMAIN[adj_x].find(P(adj_y, adj_z));
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
        var x: dynamic = L;
        while ((x <= R))
        {
          if ((info_REMAIN[x].size() == 0))
          {
            x += 1;
            continue;
          }
          {
            var loop: dynamic = 0;
            while ((loop < info_REMAIN[x].size()))
            {
              var tmp: dynamic = info_REMAIN[x][loop];
              {
                var a: dynamic = 0;
                while ((a < 3))
                {
                  {
                    var b: dynamic = 0;
                    while ((b < 3))
                    {
                      {
                        var c: dynamic = 0;
                        while ((c < 3))
                        {
                          var count_zero: dynamic = 0;
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
                          var adj_x: dynamic = (x + diff_x[a]);
                          var adj_y: dynamic = (tmp.first + diff_y[b]);
                          var adj_z: dynamic = (tmp.second + diff_z[c]);
                          if ((rangeCheck(adj_x, adj_y, adj_z) == true))
                          {
                            var at: dynamic = REMAIN[adj_x].find(P(adj_y, adj_z));
                            if ((at == REMAIN[adj_x].end()))
                            {
                              c += 1;
                              continue;
                            }
                            var from_cpp: dynamic = REMAIN[x][tmp];
                            var to: dynamic = REMAIN[adj_x][P(adj_y, adj_z)];
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
      var num_group: dynamic = 0;
      {
        var k: dynamic = 0;
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
