// Translated from solution.cpp.

func REP(i: dynamic, s: dynamic, n: dynamic)
{
  cpp_macro("for(int i=s;i<n;i++)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include<b");
}

var cpp_3 = cpp_expression("#");

class Point
{
  var x: dynamic;
  var y: dynamic;
}

var IINF = INT_MAX;

var PATH = cpp_char("$");

var h: dynamic;

var w: dynamic;

var s_size: dynamic;

var c_size: dynamic;

var bitIndex = cpp_array(12, 12);

var c = cpp_array(12, 12);

var source = cpp_array(12);

var city = cpp_array(12);

var path = cpp_array(12);

var limit: dynamic;

var next_limit: dynamic;

var dx = [0, 1, 0, -1];

var dy = [1, 0, -1, 0];

var mini: dynamic;

var bc = cpp_array((1 << 9));

var mani = cpp_array(12);

func initial_heuristic()
{
  var sum = 0;
  return sum;
}

func get_heuristic(sp: dynamic, bitmask: dynamic)
{
  var sum = 0;
  rep(i, c_size);
  if ((!((((bitmask >> i)) & 1))))
  {
    mani[i] = min(mani[i], (abs((path[sp].x - city[i].x)) + abs((path[sp].y - city[i].y))));
    sum += mani[i];
  }
  return sum;
}

var head = cpp_array(12, 12);

var opt = cpp_array(12, 12);

var blocks: dynamic;

func print_field()
{
}

func dfs(sp: dynamic, bitmask: dynamic, prev: dynamic, step: dynamic, depth: dynamic)
{
  if (((step + ((c_size - bc[bitmask]))) >= mini))
  {
    return;
  }
  if ((sp >= s_size))
  {
    return;
  }
  if ((bitmask == ((((1 << c_size)) - 1))))
  {
    mini = step;
    return;
  }
  var next = 1;
  rep(dir, 4);
  {
    if (((prev != -1) && ((((prev + 2)) % 4) == dir)))
    {
      continue;
    }
    var nx = (path[sp].x + dx[dir]);
    var ny = (path[sp].y + dy[dir]);
    if ((((c[ny][nx] == cpp_char("P")) || (c[ny][nx] == cpp_char("#"))) || (c[ny][nx] == PATH)))
    {
      continue;
    }
    var success = true;
    var cnt = 0;
    rep(i, 4);
    {
      var nx2 = (nx + dx[i]);
      var ny2 = (ny + dy[i]);
      if (((c[ny2][nx2] == PATH) || (c[ny2][nx2] == cpp_char("P"))))
      {
        cnt += 1;
      }
    }
    if ((cnt != 1))
    {
      success = false;
    }
    if ((!success))
    {
      continue;
    }
    var pp = path[sp];
    var pc = c[ny][nx];
    var nbitmask = bitmask;
    var fin = false;
    var prec = c[ny][nx];
    if ((c[ny][nx] == cpp_char("*")))
    {
      nbitmask |= ((1 << bitIndex[ny][nx]));
      fin = true;
    }
    path[sp] = [nx, ny];
    c[ny][nx] = PATH;
    swap(head[path[sp].y][path[sp].x], head[ny][nx]);
    next = 0;
    dfs(sp, nbitmask, dir, (step + 1), (depth + 1));
    if ((prec == cpp_char("*")))
    {
      dfs((sp + 1), nbitmask, -1, (step + 1), 0);
    }
    swap(head[path[sp].y][path[sp].x], head[ny][nx]);
    path[sp] = pp;
    c[ny][nx] = pc;
    if (fin)
    {
      return;
    }
  }
  if ((((depth == 0) || (c[path[sp].y][path[sp].x] == cpp_char("P"))) || (c[path[sp].y][path[sp].x] == cpp_char("*"))))
  {
    dfs((sp + 1), bitmask, -1, step, 0);
  }
}

func compute()
{
  mini = (((h * w) - s_size) - blocks);
  var csv = 0;
  dfs(0, 0, -1, 0, 0);
  return mini;
}

func main()
{
  srand(cpp_cast(time(null)));
  rep(i, ((1 << 9)))[i] = builtin_popcount(i);
  while (cpp_comma(scanf(" %d %d", (&h), (&w)), (h | w)))
  {
    s_size = cpp_assign(c_size, "=", 0);
    blocks = 0;
    rep(i, h);
    printf("%d\n", compute());
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    mani[i] = IINF;
    rep(j, s_size)[i] = min(mani[i], (abs((city[i].x - source[j].x)) + abs((city[i].y - source[j].y))));
    rep(j, c_size);
    if ((j != i))
    {
      mani[i] = min(mani[i], (abs((city[i].x - city[j].x)) + abs((city[i].y - city[j].y))));
    }
    sum += mani[i];
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      write(c[i][j]);
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    puts("");
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      head[i][j] = false;
      scanf(" %c", (&c[i][j]));
      if ((c[i][j] == cpp_char("P")))
      {
        source[s_size] = cpp_assign(path[s_size], "=", [j, i]);
        s_size += 1;
        head[i][j] = true;
      }
      if ((c[i][j] == cpp_char("*")))
      {
        city[c_size] = [j, i];
        bitIndex[i][j] = cpp_update(c_size, "++");
      }
      if ((c[i][j] == cpp_char("#")))
      {
        blocks += 1;
      }
    }
