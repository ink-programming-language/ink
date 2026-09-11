// Translated from solution.cpp.

class node
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func node(a: dynamic, b: dynamic) -> dynamic
  {
      self->x = cpp_construct(a);
      self->y = cpp_construct(b);
    }
  func node() -> dynamic
  {
      self->x = cpp_construct(0);
      self->y = cpp_construct(0);
    }
}

func operator_less(a: dynamic, b: dynamic) -> dynamic
{
  return  ((a.x == b.x)) ? (a.y < b.y) : (a.x < b.x);
}

func main() -> dynamic
{
  var dir: dynamic = cpp_uninitialized();
  dir[cpp_char("U")] = 0;
  dir[cpp_char("D")] = 1;
  dir[cpp_char("L")] = 2;
  dir[cpp_char("R")] = 3;
  var dirdir: dynamic = [[0, 1], [0, -1], [-1, 0], [1, 0]];
  var des: dynamic = cpp_uninitialized();
  read(des.x, des.y);
  var s: dynamic = cpp_uninitialized();
  read(s);
  var scope: dynamic = cpp_uninitialized();
  scope.insert(node(0, 0));
  var move: dynamic = cpp_construct(0, 0);
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      move.x += dirdir[dir[s[i]]][0];
      move.y += dirdir[dir[s[i]]][1];
      scope.insert(move);
      i += 1;
    }
  }
  var source: dynamic = cpp_construct(0, 0);
  var flag: dynamic = false;
  {
    var it: dynamic = scope.begin();
    while ((it != scope.end()))
    {
      if (((move.x == 0) && (move.y == 0)))
      {
        flag |= (((((*it)).x == des.x) && (((*it)).y == des.y)));
      } else if (((move.x == 0) && (move.y != 0)))
      {
        flag |= ((((((*it)).x == des.x) && ((((des.y - ((*it)).y)) % move.y) == 0)) && (((des.y * 1) * move.y) >= 0)));
      } else if (((move.x != 0) && (move.y == 0)))
      {
        flag |= ((((((*it)).y == des.y) && ((((des.x - ((*it)).x)) % move.x) == 0)) && (((des.x * 1) * move.x) >= 0)));
      } else if ((((((((((des.x - ((*it)).x)) * 1) * move.y) == ((((des.y - ((*it)).y)) * 1) * move.x))) && ((((des.x - ((*it)).x)) % move.x) == 0)) && ((((des.y - ((*it)).y)) % move.y) == 0)) && (((((des.x - ((*it)).x)) * 1) * move.x) >= 0)))
      {
        flag = true;
      }
      if (flag)
      {
        break;
      }
      it += 1;
    }
  }
  if (flag)
  {
    write("Yes", "\n");
  } else
  {
    write("No", "\n");
  }
  return 0;
}
