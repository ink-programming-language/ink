// Translated from solution.cpp.

func LOG()
{
  return cpp_expression("/* * * */ #include <bits");
}

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i = (int)(a); i < (int)(b); ++i)");
}

func RFOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i = (int)(b - 1); i >= (int)(a); --i)");
}

func REP(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = 0; i < (int)(n); ++i)");
}

func RREP(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = (int)(n - 1); i >= 0; --i)");
}

func ALL(a: dynamic)
{
  return cpp_expression("/* * * */ #include");
}

func RALL(a: dynamic)
{
  return cpp_expression("/* * * */ #include <");
}

func EXIST(s: dynamic, e: dynamic)
{
  return cpp_expression("/* * * */ #include <bi");
}

func SORT(c: dynamic)
{
  return cpp_expression("/* * * */");
}

func RSORT(c: dynamic)
{
  return cpp_expression("/* * * */");
}

func SQ(n: dynamic)
{
  return cpp_expression("/* * *");
}

var dx = [-1, 0, 1, 0];

var dy = [0, -1, 0, 1];

func main()
{
  var way = "<^>v";
  var h: dynamic;
  var w: dynamic;
  read(h, w);
  var ox: dynamic;
  var oy: dynamic;
  var ix: dynamic;
  var iy: dynamic;
  var id: dynamic;
  var d = cpp_array(4, w, h);
  fill_n(cpp_cast(d), ((h * w) * 4), false);
  var number: dynamic;
  while (true)
  {
    if (d[iy][ix][id])
    {
      write(-1, "\n");
      break;
    }
    if (((ix == ox) && (iy == oy)))
    {
      write((number.size() + 1), "\n");
      break;
    }
    d[iy][ix][id] = true;
    number.insert(((iy * w) + ix));
    var rd = (((id + 1)) % 4);
    var rx = (ix + dx[rd]);
    var ry = (iy + dy[rd]);
    if ((((((0 <= rx) && (rx < w)) && (0 <= ry)) && (ry < h)) && (field[ry][rx] != cpp_char("#"))))
    {
      id = rd;
      ix = rx;
      iy = ry;
      continue;
    }
    var sx = (ix + dx[id]);
    var sy = (iy + dy[id]);
    if ((((((0 <= sx) && (sx < w)) && (0 <= sy)) && (sy < h)) && (field[sy][sx] != cpp_char("#"))))
    {
      ix = sx;
      iy = sy;
      continue;
    }
    id = (((id + 3)) % 4);
  }
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
      var c: dynamic;
      read(c);
      var __cpp_switch_1 = c;
      if (__cpp_switch_1 == cpp_char("G"))
      {
        ox = x;
        oy = y;
        break;
      }
      else if (__cpp_switch_1 == cpp_char("<"))
      {
      }
      else if (__cpp_switch_1 == cpp_char("^"))
      {
      }
      else if (__cpp_switch_1 == cpp_char(">"))
      {
      }
      else if (__cpp_switch_1 == cpp_char("v"))
      {
        ix = x;
        iy = y;
        id = way.find(c);
        break;
      }
      field[y][x] = c;
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
  }
