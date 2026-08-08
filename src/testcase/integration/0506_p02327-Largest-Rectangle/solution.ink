// Translated from solution.cpp.

var MAX = 1400;

var H: dynamic;

var W: dynamic;

var buffer = cpp_array(MAX, MAX);

var T = cpp_array(MAX, MAX);

class Rectangle
{
  var height: dynamic;
  var pos: dynamic;
}

func getLargestRectangle(size: dynamic, buffer: dynamic)
{
  var S: dynamic;
  var maxv = 0;
  buffer[size] = 0;
  {
    var i = 0;
    while ((i <= size))
    {
      var rect: dynamic;
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
          var target = i;
          while (((!S.empty()) && (S.top().height >= rect.height)))
          {
            var pre = S.top();
            S.pop();
            var area = (pre.height * ((i - pre.pos)));
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

func getLargestRectangle()
{
  {
    var j = 0;
    while ((j < W))
    {
      {
        var i = 0;
        while ((i < H))
        {
          if (buffer[i][j])
          {
            T[i][j] = 0;
          } else
          {
            T[i][j] = if (((i > 0))) (T[(i - 1)][j] + 1) else 1;
          }
          i += 1;
        }
      }
      j += 1;
    }
  }
  var maxv = 0;
  {
    var i = 0;
    while ((i < H))
    {
      maxv = max(maxv, getLargestRectangle(W, T[i]));
      i += 1;
    }
  }
  return maxv;
}

func main(argument_0: dynamic)
{
  scanf("%d %d", (&H), (&W));
  {
    var i = 0;
    while ((i < H))
    {
      {
        var j = 0;
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
