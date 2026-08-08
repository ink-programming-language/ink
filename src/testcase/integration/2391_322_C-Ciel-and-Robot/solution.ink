// Translated from solution.cpp.

class node
{
  var x: dynamic;
  var y: dynamic;
  func node(a: dynamic, b: dynamic)
  {
      this->x = cpp_construct(a);
      this->y = cpp_construct(b);
    }
  func node()
  {
      this->x = cpp_construct(0);
      this->y = cpp_construct(0);
    }
}

func operator_less(a: dynamic, b: dynamic)
{
  return if ((a.x == b.x)) (a.y < b.y) else (a.x < b.x);
}

func main()
{
  var dir: dynamic;
  dir[cpp_char("U")] = 0;
  dir[cpp_char("D")] = 1;
  dir[cpp_char("L")] = 2;
  dir[cpp_char("R")] = 3;
  var dirdir = [[0, 1], [0, -1], [-1, 0], [1, 0]];
  var des: dynamic;
  read(des.x, des.y);
  var s: dynamic;
  read(s);
  var scope: dynamic;
  scope.insert(node(0, 0));
  var move = cpp_construct(0, 0);
  {
    var i = 0;
    while ((i < s.size()))
    {
      move.x += dirdir[dir[s[i]]][0];
      move.y += dirdir[dir[s[i]]][1];
      scope.insert(move);
      i += 1;
    }
  }
  var source = cpp_construct(0, 0);
  var flag = false;
  {
    var it = scope.begin();
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
