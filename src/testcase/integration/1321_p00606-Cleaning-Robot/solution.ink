// Translated from solution.cpp.

func toInt(s: dynamic) -> dynamic
{
  var v: dynamic = cpp_uninitialized();
  (sin >> v);
  return v;
}

func toString(x: dynamic) -> dynamic
{
  var sout: dynamic = cpp_uninitialized();
  (sout << x);
  return sout.str();
}

func ALL(a: dynamic) -> dynamic
{
  return cpp_expression("#include <vector> #in");
}

func RALL(a: dynamic) -> dynamic
{
  return cpp_expression("#include <vector> #incl");
}

func EXIST(s: dynamic, e: dynamic) -> dynamic
{
  return cpp_expression("#include <vector> #inclu");
}

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func REP(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

func EACH(t: dynamic, i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(t::iterator i=(c).begin(); i!=(c).end(); ++i)");
}

var EPS: dynamic = 1e-10;

var PI: dynamic = acos(-1.0);

var dx: dynamic = [-1, 0, 1, 0];

var dy: dynamic = [0, -1, 0, 1];

var roomChar: dynamic = [[cpp_char("A"), cpp_char("B"), cpp_char("C")], [cpp_char("D"), cpp_char("E"), cpp_char("F")], [cpp_char("G"), cpp_char("H"), cpp_char("I")]];

func getRoomChar(x: dynamic, y: dynamic) -> dynamic
{
  return roomChar[y][x];
}

func getRoomLoc(c: dynamic) -> dynamic
{
  cpp_statement("REP(x,3)");
  {
    cpp_statement("REP(y,3)");
    {
      if ((roomChar[y][x] == c))
      {
        return make_pair(x, y);
      }
    }
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  while (cpp_comma((cin >> n), n))
  {
    var s: dynamic = cpp_uninitialized();
    var t: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    read(s, t, b);
    var p: dynamic = cpp_construct((n + 1), vvd(3, vd(3)));
    var sl: dynamic = getRoomLoc(s);
    p[0][sl.first][sl.second] = 1;
    var tl: dynamic = getRoomLoc(t);
    printf("%.10f\n", p[n][tl.first][tl.second]);
  }
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      cpp_statement("REP(x,3)");
      {
        cpp_statement("REP(y,3)");
        {
          cpp_statement("REP(d,4)");
          {
            var xx: dynamic = (x + dx[d]);
            var yy: dynamic = (y + dy[d]);
            if ((((((xx >= 0) && (yy >= 0)) && (xx < 3)) && (yy < 3)) && (getRoomChar(xx, yy) != b)))
            {
              p[(i + 1)][xx][yy] += (p[i][x][y] / 4);
            } else
            {
              p[(i + 1)][x][y] += (p[i][x][y] / 4);
            }
          }
        }
      }
    }
