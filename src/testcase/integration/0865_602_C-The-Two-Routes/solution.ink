// Translated from solution.cpp.

var roads: dynamic = cpp_array(405, 405);

func bfs(n: dynamic, road_type: dynamic) -> dynamic
{
  var towns: dynamic = cpp_uninitialized();
  towns.push(1);
  var visited: dynamic = cpp_array(405);
  var road_length: dynamic = cpp_array(405);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      visited[i] = false;
      road_length[i] = -1;
      i += 1;
    }
  }
  road_length[1] = 0;
  visited[1] = true;
  while ((!towns.empty()))
  {
    var current: dynamic = towns.front();
    towns.pop();
    {
      var town: dynamic = 1;
      while ((town <= n))
      {
        if (((roads[current][town] == road_type) && (!visited[town])))
        {
          visited[town] = true;
          towns.push(town);
          road_length[town] = (road_length[current] + 1);
        }
        town += 1;
      }
    }
  }
  return road_length[n];
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  while (cpp_update(m, "--"))
  {
    read(x, y);
    roads[x][y] = true;
    roads[y][x] = true;
  }
  var ans1: dynamic = bfs(n, false);
  var ans2: dynamic = bfs(n, true);
  if (((ans1 == -1) || (ans2 == -1)))
  {
    write("-1", "\n");
  } else
  {
    write(max(ans1, ans2), "\n");
  }
  return 0;
}
