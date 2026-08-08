// Translated from solution.cpp.

func REP(i: dynamic, s: dynamic, n: dynamic)
{
  cpp_macro("for(int i=s;i<n;i++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include<b");
}

var H: dynamic;

var W: dynamic;

var LP: dynamic;

var LP_dir: dynamic;

var polluted_room = cpp_array(9, 9);

var visited = cpp_array(9, 9);

var room = cpp_array(9, 9);

var dx8 = [+0, +1, +1, +1, +0, -1, -1, -1];

var dy8 = [-1, -1, +0, +1, +1, +1, +0, -1];

var dx = [0, 1, 0, -1];

var dy = [1, 0, -1, 0];

var statue: dynamic;

func isMirror(x: dynamic, y: dynamic)
{
  return ((((((room[y][x] == cpp_char("/")) || (room[y][x] == cpp_char("\\"))) || (room[y][x] == cpp_char("-"))) || (room[y][x] == cpp_char("|"))) || (room[y][x] == cpp_char("O"))));
}

func isValid(x: dynamic, y: dynamic)
{
  return ((((0 <= x) && (x < W)) && (0 <= y)) && (y < H));
}

func validMove(x: dynamic, y: dynamic, dir: dynamic)
{
  if ((((((room[y][x] == cpp_char("#")) || (room[y][x] == cpp_char("*"))) || (room[y][x] == cpp_char("S"))) || (room[y][x] == cpp_char("L"))) || (room[y][x] == cpp_char("D"))))
  {
    return false;
  }
  if (isMirror(x, y))
  {
    var nx = (x + dx[dir]);
    var ny = (y + dy[dir]);
    if ((!isValid(nx, ny)))
    {
      return false;
    }
    if ((room[ny][nx] != cpp_char(".")))
    {
      return false;
    }
  }
  return true;
}

func UruruBeam(sp: dynamic, sp_dir: dynamic)
{
  cpp_statement("rep(i,H) rep(j,W)");
  polluted_room[i][j] = false;
  var deq: dynamic;
  deq.push_back(ii(sp, sp_dir));
  polluted_room[(sp / W)][(sp % W)] = true;
  var S: dynamic;
  S.insert(ii(sp, sp_dir));
  while ((!deq.empty()))
  {
    var tmp = deq.front();
    deq.pop_front();
    var x = (tmp.first % W);
    var y = (tmp.first / W);
    var dir = tmp.second;
    while (1)
    {
      x += dx8[dir];
      y += dy8[dir];
      if ((!isValid(x, y)))
      {
        break;
      }
      polluted_room[y][x] = true;
      if (((((room[y][x] == cpp_char("#")) || (room[y][x] == cpp_char("*"))) || (room[y][x] == cpp_char("D"))) || (room[y][x] == cpp_char("L"))))
      {
        break;
      }
      if ((room[y][x] == cpp_char("S")))
      {
        if ((dir & 1))
        {
          polluted_room[y][x] = false;
        }
        break;
      }
      if ((room[y][x] == cpp_char("/")))
      {
        if (((dir == 1) || (dir == 5)))
        {
          break;
        }
        var next_dir = -1;
        if ((dir == 0))
        {
          next_dir = 2;
        } else if ((dir == 2))
        {
          next_dir = 0;
        } else if ((dir == 3))
        {
          next_dir = 7;
        } else if ((dir == 4))
        {
          next_dir = 6;
        } else if ((dir == 6))
        {
          next_dir = 4;
        }
        var next = ii((x + (y * W)), next_dir);
        if (S.count(next))
        {
          break;
        }
        S.insert(next);
        deq.push_back(next);
        break;
      } else if ((room[y][x] == cpp_char("\\")))
      {
        if (((dir == 3) || (dir == 7)))
        {
          break;
        }
        var next_dir = -1;
        if ((dir == 0))
        {
          next_dir = 6;
        } else if ((dir == 1))
        {
          next_dir = 5;
        } else if ((dir == 2))
        {
          next_dir = 4;
        } else if ((dir == 4))
        {
          next_dir = 2;
        } else if ((dir == 5))
        {
          next_dir = 1;
        } else if ((dir == 6))
        {
          next_dir = 0;
        }
        var next = ii((x + (y * W)), next_dir);
        if (S.count(next))
        {
          break;
        }
        S.insert(next);
        deq.push_back(next);
        break;
      } else if ((room[y][x] == cpp_char("-")))
      {
        if (((dir == 2) || (dir == 6)))
        {
          break;
        }
        var next_dir = -1;
        if ((dir == 0))
        {
          next_dir = 4;
        } else if ((dir == 1))
        {
          next_dir = 3;
        } else if ((dir == 3))
        {
          next_dir = 1;
        } else if ((dir == 4))
        {
          next_dir = 0;
        } else if ((dir == 5))
        {
          next_dir = 7;
        } else if ((dir == 7))
        {
          next_dir = 5;
        }
        var next = ii((x + (y * W)), next_dir);
        if (S.count(next))
        {
          break;
        }
        S.insert(next);
        deq.push_back(next);
        break;
      } else if ((room[y][x] == cpp_char("|")))
      {
        if (((dir == 0) || (dir == 4)))
        {
          break;
        }
        var next_dir = -1;
        if ((dir == 1))
        {
          next_dir = 7;
        } else if ((dir == 2))
        {
          next_dir = 6;
        } else if ((dir == 3))
        {
          next_dir = 5;
        } else if ((dir == 5))
        {
          next_dir = 3;
        } else if ((dir == 6))
        {
          next_dir = 2;
        } else if ((dir == 7))
        {
          next_dir = 1;
        }
        var next = ii((x + (y * W)), next_dir);
        if (S.count(next))
        {
          break;
        }
        S.insert(next);
        deq.push_back(next);
        break;
      } else if ((room[y][x] == cpp_char("O")))
      {
        var next = [ii((x + (y * W)), -1), ii((x + (y * W)), -1)];
        if ((dir == 0))
        {
          next[0].second = 1;
          next[1].second = 7;
        } else if ((dir == 1))
        {
          next[0].second = 0;
          next[1].second = 2;
        } else if ((dir == 2))
        {
          next[0].second = 1;
          next[1].second = 3;
        } else if ((dir == 3))
        {
          next[0].second = 2;
          next[1].second = 4;
        } else if ((dir == 4))
        {
          next[0].second = 3;
          next[1].second = 5;
        } else if ((dir == 5))
        {
          next[0].second = 4;
          next[1].second = 6;
        } else if ((dir == 6))
        {
          next[0].second = 7;
          next[1].second = 5;
        } else if ((dir == 7))
        {
          next[0].second = 0;
          next[1].second = 6;
        }
        rep(i, 2);
        {
          if ((!S.count(next[i])))
          {
            S.insert(next[i]);
            deq.push_back(next[i]);
          }
        }
        break;
      }
    }
  }
}

func solved(sp: dynamic)
{
  rep(i, cpp_cast(statue.size()));
  if ((!polluted_room[(statue[i] / W)][(statue[i] % W)]))
  {
    return false;
  }
  rep(i, H);
  rep(j, W)[i][j] = false;
  var deq: dynamic;
  deq.push_back(sp);
  visited[(sp / W)][(sp % W)] = true;
  while ((!deq.empty()))
  {
    var cur = deq.front();
    deq.pop_front();
    if (polluted_room[(cur / W)][(cur % W)])
    {
      continue;
    }
    if ((room[(cur / W)][(cur % W)] == cpp_char("D")))
    {
      return true;
    }
    rep(i, 4);
    {
      var nx = ((cur % W) + dx[i]);
      var ny = ((cur / W) + dy[i]);
      if ((!isValid(nx, ny)))
      {
        continue;
      }
      if ((!(((room[ny][nx] == cpp_char(".")) || (room[ny][nx] == cpp_char("D"))))))
      {
        continue;
      }
      if ((polluted_room[ny][nx] || visited[ny][nx]))
      {
        continue;
      }
      visited[ny][nx] = true;
      deq.push_back((nx + (ny * W)));
    }
  }
  return false;
}

var S: dynamic;

func BackTracking(cur: dynamic, prev: dynamic, deq: dynamic)
{
  if ((!deq.empty()))
  {
    sort(deq.begin(), deq.end());
  }
  var tmper: dynamic;
  rep(i, deq.size()).push_back(((deq[i] * 1000) + room[(deq[i] / W)][(deq[i] % W)]));
  if (S.count(ivi(cur, tmper)))
  {
    return false;
  }
  S.insert(ivi(cur, tmper));
  UruruBeam(LP, LP_dir);
  if (polluted_room[(cur / W)][(cur % W)])
  {
    return false;
  }
  if (solved(cur))
  {
    return true;
  }
  rep(i, 4);
  {
    if ((prev == (((i + 2)) % 4)))
    {
      continue;
    }
    var nx = ((cur % W) + dx[i]);
    var ny = ((cur / W) + dy[i]);
    if ((!isValid(nx, ny)))
    {
      continue;
    }
    if ((!validMove(nx, ny, i)))
    {
      continue;
    }
    var mirror = false;
    var new_one = -1;
    if (isMirror(nx, ny))
    {
      rep(j, deq.size());
      if ((((deq[j] % W) == nx) && ((deq[j] / W) == ny)))
      {
        new_one = j;
        break;
      }
      if (((new_one == -1) && (deq.size() == 2)))
      {
        continue;
      }
      assert((room[(ny + dy[i])][(nx + dx[i])] == cpp_char(".")));
      if ((new_one == -1))
      {
        deq.push_back((((nx + dx[i])) + (((ny + dy[i])) * W)));
      } else
      {
        deq[new_one] = (((nx + dx[i])) + (((ny + dy[i])) * W));
      }
      room[(ny + dy[i])][(nx + dx[i])] = room[ny][nx];
      room[ny][nx] = cpp_char(".");
      mirror = true;
      UruruBeam(LP, LP_dir);
      if (polluted_room[ny][nx])
      {
        cpp_goto("goto Skip;");
      }
    }
    if (((!mirror) && polluted_room[ny][nx]))
    {
      continue;
    }
    if (BackTracking((nx + (ny * W)), (if (mirror) -1 else i), deq))
    {
      return true;
    }
    if (mirror)
    {
      assert(isMirror((nx + dx[i]), (ny + dy[i])));
      room[ny][nx] = room[(ny + dy[i])][(nx + dx[i])];
      room[(ny + dy[i])][(nx + dx[i])] = cpp_char(".");
      if ((new_one == -1))
      {
        rep(j, deq.size());
        if ((deq[j] == (((nx + dx[i])) + (((ny + dy[i])) * W))))
        {
          deq.erase((deq.begin() + j));
          break;
        }
      } else
      {
        rep(j, deq.size());
        if ((deq[j] == (((nx + dx[i])) + (((ny + dy[i])) * W))))
        {
          deq[j] = (nx + (ny * W));
          break;
        }
      }
    }
  }
  return false;
}

func main()
{
  LP = cpp_assign(LP_dir, "=", -1);
  var sp = -1;
  read(W, H);
  rep(i, H);
  {
    var k = 0;
    while ((k < 8))
    {
      var nx = ((LP % W) + dx8[k]);
      var ny = ((LP / W) + dy8[k]);
      if ((!isValid(nx, ny)))
      {
        k += 2;
        continue;
      }
      if (((((nx == 0) || (ny == 0)) || (nx == (W - 1))) || (ny == (H - 1))))
      {
        k += 2;
        continue;
      }
      LP_dir = k;
      k += 2;
    }
  }
  assert((LP_dir != -1));
  UruruBeam(LP, LP_dir);
  if (polluted_room[(sp / W)][(sp % W)])
  {
    puts("No");
    return 0;
  }
  if (solved(sp))
  {
    puts("Yes");
    return 0;
  }
  var deq: dynamic;
  puts(if (BackTracking(sp, -1, deq)) "Yes" else "No");
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    read(room[i][j]);
    if ((room[i][j] == cpp_char("@")))
    {
      sp = (j + (i * W));
      room[i][j] = cpp_char(".");
    }
    if ((room[i][j] == cpp_char("L")))
    {
      assert((LP == -1));
      LP = (j + (i * W));
    }
    if ((room[i][j] == cpp_char("S")))
    {
      statue.push_back((j + (i * W)));
    }
  }
