// Translated from solution.cpp.

var MAX: dynamic = 1400;

var H: dynamic = cpp_uninitialized();

var W: dynamic = cpp_uninitialized();

var buffer: dynamic = cpp_array(MAX, MAX);

var T: dynamic = cpp_array(MAX, MAX);

class Rectangle
{
  var height: dynamic = cpp_uninitialized();
  var pos: dynamic = cpp_uninitialized();
}

func getLargestRectangle(size: dynamic, buffer: dynamic) -> dynamic
{
  var S: dynamic = cpp_uninitialized();
  var maxv: dynamic = 0;
  buffer[size] = 0;
  {
    var i: dynamic = 0;
    while ((i <= size))
    {
      var rect: dynamic = cpp_uninitialized();
      rect.height = buffer[i];
      rect.pos = i;
      if (S.empty())
      {
        S.push(rect);
      } else
      {
        if ((S.top().height < rect.height))
        {
          S.push(rect);
        } else if ((S.top().height > rect.height))
        {
          var target: dynamic = i;
          while (((!S.empty()) && (S.top().height >= rect.height)))
          {
            var pre: dynamic = S.top();
            S.pop();
            var area: dynamic = (pre.height * ((i - pre.pos)));
            maxv = max(maxv, area);
            target = pre.pos;
          }
          rect.pos = target;
          S.push(rect);
        }
      }
      i += 1;
    }
  }
  return maxv;
}

func getLargestRectangle() -> dynamic
{
  {
    var j: dynamic = 0;
    while ((j < W))
    {
      {
        var i: dynamic = 0;
        while ((i < H))
        {
          if (buffer[i][j])
          {
            T[i][j] = 0;
          } else
          {
            T[i][j] =  (((i > 0))) ? (T[(i - 1)][j] + 1) : 1;
          }
          i += 1;
        }
      }
      j += 1;
    }
  }
  var maxv: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < H))
    {
      maxv = max(maxv, getLargestRectangle(W, T[i]));
      i += 1;
    }
  }
  return maxv;
}

func main(argument_0: dynamic) -> dynamic
{
  scanf("%d %d", (&H), (&W));
  {
    var i: dynamic = 0;
    while ((i < H))
    {
      {
        var j: dynamic = 0;
        while ((j < W))
        {
          scanf("%d", (&buffer[i][j]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(getLargestRectangle(), "\n");
  return 0;
}
