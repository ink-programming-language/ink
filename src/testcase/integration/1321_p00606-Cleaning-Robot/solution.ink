// Translated from solution.cpp.

func toInt(s: dynamic)
{
  var v: dynamic;
  (sin >> v);
  return v;
}

func toString(x: dynamic)
{
  var sout: dynamic;
  (sout << x);
  return sout.str();
}

func ALL(a: dynamic)
{
  return cpp_expression("#include <vector> #in");
}

func RALL(a: dynamic)
{
  return cpp_expression("#include <vector> #incl");
}

func EXIST(s: dynamic, e: dynamic)
{
  return cpp_expression("#include <vector> #inclu");
}

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <");
}

func EACH(t: dynamic, i: dynamic, c: dynamic)
{
  cpp_macro("for(t::iterator i=(c).begin(); i!=(c).end(); ++i)");
}

var EPS = 1e-10;

var PI = acos(-1.0);

var dx = [-1, 0, 1, 0];

var dy = [0, -1, 0, 1];

var roomChar = [[cpp_char("A"), cpp_char("B"), cpp_char("C")], [cpp_char("D"), cpp_char("E"), cpp_char("F")], [cpp_char("G"), cpp_char("H"), cpp_char("I")]];

func getRoomChar(x: dynamic, y: dynamic)
{
  return roomChar[y][x];
}

func getRoomLoc(c: dynamic)
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

func main()
{
  var n: dynamic;
  while (cpp_comma((cin >> n), n))
  {
    var s: dynamic;
    var t: dynamic;
    var b: dynamic;
    read(s, t, b);
    var p = cpp_construct((n + 1), vvd(3, vd(3)));
    var sl = getRoomLoc(s);
    p[0][sl.first][sl.second] = 1;
    var tl = getRoomLoc(t);
    printf("%.10f\n", p[n][tl.first][tl.second]);
  }
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
      cpp_statement("REP(x,3)");
      {
        cpp_statement("REP(y,3)");
        {
          cpp_statement("REP(d,4)");
          {
            var xx = (x + dx[d]);
            var yy = (y + dy[d]);
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
