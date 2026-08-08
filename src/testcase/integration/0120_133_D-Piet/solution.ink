// Translated from solution.cpp.

var INF = 500000000;

func debug(a: dynamic, b: dynamic)
{
  {
    while ((a != b))
    {
      write((*a), cpp_char(" "));
      a += 1;
    }
  }
  write("\n");
}

var w: dynamic;

var h: dynamic;

var n: dynamic;

var buf = cpp_array(55, 55);

var moveTo = cpp_array(2, 4, 55, 55);

var state = cpp_array(2, 4, 55, 55);

var dx = [1, 0, -1, 0];

var dy = [0, 1, 0, -1];

func main()
{
  read(h, n);
  {
    var i = 0;
    while ((i < h))
    {
      read(buf[i]);
      i += 1;
    }
  }
  w = strlen(buf[0]);
  {
    var i = 0;
    while ((i < h))
    {
      {
        var j = 0;
        while ((j < w))
        {
          {
            var k = 0;
            while ((k < 4))
            {
              {
                var l = 0;
                while ((l < 2))
                {
                  var cx = j;
                  var cy = i;
                  while (1)
                  {
                    var px = (cx + dx[k]);
                    var py = (cy + dy[k]);
                    if ((((((px < 0) || (py < 0)) || (px >= w)) || (py >= h)) || (buf[py][px] != buf[cy][cx])))
                    {
                      break;
                    }
                    cx = px;
                    cy = py;
                  }
                  var add: dynamic;
                  if ((l == 0))
                  {
                    add = 3;
                  } else
                  {
                    add = 1;
                  }
                  var k2 = (((k + add)) % 4);
                  var px: dynamic;
                  var py: dynamic;
                  while (1)
                  {
                    px = (cx + dx[k2]);
                    py = (cy + dy[k2]);
                    if ((((((px < 0) || (py < 0)) || (px >= w)) || (py >= h)) || (buf[py][px] != buf[cy][cx])))
                    {
                      break;
                    }
                    cx = px;
                    cy = py;
                  }
                  px = (cx + dx[k]);
                  py = (cy + dy[k]);
                  if ((((((px < 0) || (py < 0)) || (px >= w)) || (py >= h)) || (buf[py][px] == cpp_char("0"))))
                  {
                    state[i][j][k][l] = 1;
                    moveTo[i][j][k][l] = make_pair(cy, cx);
                  } else
                  {
                    moveTo[i][j][k][l] = make_pair(py, px);
                  }
                  l += 1;
                }
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var cx = 0;
  var cy = 0;
  var dir = 0;
  var hand = 0;
  {
    var hoge = 0;
    while ((hoge < n))
    {
      var nxt = moveTo[cy][cx][dir][hand];
      var flag = state[cy][cx][dir][hand];
      cy = nxt.first;
      cx = nxt.second;
      if (flag)
      {
        if ((hand == 0))
        {
          hand = 1;
        } else
        {
          hand = 0;
          dir = (((dir + 1)) % 4);
        }
      }
      hoge += 1;
    }
  }
  printf("%c\n", buf[cy][cx]);
  return 0;
}
