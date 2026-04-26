/* pipeline/generated.h — declaration for committed/generated pipelines */
#ifndef IMGENGINE_PIPELINE_GENERATED_H
#define IMGENGINE_PIPELINE_GENERATED_H

#ifdef __cplusplus
extern "C" {
#endif

/* Register pipelines generated and committed into the tree.
 * This is intentionally explicit to avoid constructor/init-order issues.
 */
void register_generated_pipelines(void);

#ifdef __cplusplus
}
#endif

#endif /* IMGENGINE_PIPELINE_GENERATED_H */
