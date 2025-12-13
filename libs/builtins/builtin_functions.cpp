namespace builtins {

model::Object* print(model::Object* self, const model::List* args) {
    std::string text;
    for (const auto* arg : args->val) {
        text += arg->to_string() + " ";
    }
    std::cout << text << std::endl;
    return new model::Nil();
};

model::Object* input(model::Object* self, const model::List* args) {
    const auto prompt_obj = get_one_arg(args);
    std::cout << prompt_obj->to_string();
    std::string result;
    std::getline(std::cin, result);
    return new model::String(result);
};

model::Object* isinstance(model::Object* self, const model::List* args) {
    if (!(args->val.size() == 2)) {
        assert(false && "函数参数不足两个");
    }

    const auto a = args->val[0];
    const auto b = args->val[1];
    return check_based_object(a, b);
    
};

model::Object* help(model::Object* self, const model::List* args) {
    // todo
    return new Nil();
};

model::Object* breakpointer(model::Object* self, const model::List* args) {
    // todo
    return new Nil();
};

model::Object* range(model::Object* self, const model::List* args) {
    // todo
    return new Nil();
};

model::Object* cmd(model::Object* self, const model::List* args) {
    // todo
    return new Nil();
};

model::Object* now(model::Object* self, const model::List* args) {
    // todo
    return new Nil();
};

model::Object* setattr(model::Object* self, const model::List* args) {
    // todo
    return new Nil();
};

model::Object* getattr(model::Object* self, const model::List* args) {
    // todo
    return new Nil();
};

model::Object* delattr(model::Object* self, const model::List* args) {
    // todo
    return new Nil();
};

model::Object* getrefc(model::Object* self, const model::List* args) {
    // todo
    return new Nil();
};

model::Object* copy(model::Object* self, const model::List* args) {
    // todo
    return new Nil();
};

model::Object* create(model::Object* self, const model::List* args) {
    // todo
    return new Nil();
};

model::Object* typeofobj(model::Object* self, const model::List* args) {
    // todo
    return new Nil();
};

}